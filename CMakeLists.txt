cmake_minimum_required(VERSION 3.0)

project(
  H2X
  DESCRIPTION "H2: It is a sample hypervisor to support Hexagon Linux"
  LANGUAGES C ASM)
# -------------------------------------------------------------------
#
# Default architecture and address settings.
#    - These are the most likely values to change between targets
#    - This settings should be globally respected.
#
set(ARCHV
    "68"
    CACHE STRING
          "Only 68 is known to work but this can be overridden during cmake configuration, -DARCHV=##")

set(LINUX_LOAD_ADDRESS
    "0xA0000000"
    CACHE STRING
          "Set on the cmake config line: -DLINUX_LOAD_ADDRESS=0x..
          This is where the linix binary is expected to be found.
          For QEMU the option would be:
           -device \"loader,addr=0xA0000000,file=LINUX/obj_qemu_ex/vmlinux.bin\"
          Assuming LINUX_LOAD_ADDRESS was 0xA0000000:")

set(BOOTVM_LOAD_ADDRESS
    "0x9B800000"
    CACHE STRING
          "This is the physical address the reset of the system expects firmware to reside.")

set(BOOTVM_H2_OFFSET
    "0x400000"
    CACHE STRING
          "-DH2K_BOOTVM_OFFSET=0x400000")

set(H2K_LINK_ADDR
    "0xff000000"
    CACHE STRING
          "This is the virtual address the reset of the system expects firmware to reside.")
math(EXPR  H2K_BOOT_VM   ${H2K_LINK_ADDR}+${BOOTVM_H2_OFFSET} OUTPUT_FORMAT HEXADECIMAL)
math(EXPR  H2K_GUEST_END ${H2K_LINK_ADDR}+0x1000000 OUTPUT_FORMAT HEXADECIMAL)


math(EXPR BOOTVM_LOADLINUX_ADDRESS ${BOOTVM_LOAD_ADDRESS}+${BOOTVM_H2_OFFSET} OUTPUT_FORMAT HEXADECIMAL)

# -------------------------------------------------------------------
# The compile_commands.json file is comsumed by IDE's to support
# code browsing.  These files are created in the build directory and
# we are setting a link to the source directory to support "out-of-tree"
# builds.

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)

# -------------------------------------------------------------------
# Common defines and options.
#    NOTE:
#        "NO_PRINT" add this to remove bootup version info
#        "NO_LOAD"  add this if the image is loaded elsewhere.
#                   QEMU:    NO_LOAD should be set.
#                   QQVP/HW: NO_LOAD should not be set.

add_compile_definitions (
          "ARCHV=${ARCHV}"
          "H2K_KERNEL_PGSIZE=5"
          "H2K_KERNEL_ADDRBITS=12"
          "LINUX_LINK_ADDR=${LINUX_LOAD_ADDRESS}"
          "LINUX_LOAD_ADDR=${LINUX_LOAD_ADDRESS}"
          "NO_LOAD"
)

add_compile_options (
          "-G0"
          "-mv${ARCHV}"
          "-g"
          "-O2"
          "-Wall"
          "-Werror"
)

#
# Make a hex value out of the current git-sha to use as the build-id
#
execute_process(
        COMMAND "git" rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE)
set (GIT_COMMIT "0x${GIT_COMMIT}")


# -------------------------------------------------------------------
# These are the only directories built.
#
add_subdirectory(kernel)
add_subdirectory(libs)
add_subdirectory(booter)
add_subdirectory(linux)

# -------------------------------------------------------------------
#
# Prepare the tree for building.
#   - Stuff headers into a couple of include directories, this follows what
#     the original Makefiles did.
#   - Each execute_process is to be done in sequential order so the
#     order below is important
#   - DEBUG
#       Add "COMMAND_ECHO STDOUT" to the execute_process you want to look at.
#
file (GLOB KERNEL_HDR ${CMAKE_SOURCE_DIR}/kernel/*/*/*.h)
file (GLOB H2_HDR
    ${CMAKE_SOURCE_DIR}/libs/h2/*/h2*.h
    ${CMAKE_SOURCE_DIR}/libs/h2_compat/*/*.h
    ${CMAKE_SOURCE_DIR}/libs/posix/pthread/*/*.h
    ${CMAKE_SOURCE_DIR}/libs/syscall/angel/include/*.h
  )

execute_process(
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/kernel/include
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include
  COMMAND cp ${KERNEL_HDR} ${CMAKE_CURRENT_BINARY_DIR}/kernel/include
  COMMAND cp ${H2_HDR} ${CMAKE_CURRENT_BINARY_DIR}/include
  )

execute_process(
        COMMAND  ${CMAKE_C_COMPILER} -I ${CMAKE_CURRENT_BINARY_DIR}/kernel/include -I ${CMAKE_CURRENT_BINARY_DIR}/include -o ${CMAKE_CURRENT_BINARY_DIR}/offsets kernel/build/offsets/offsets.ref.c
)
execute_process(
        COMMAND  ${HEXAGON_SIMULATOR} --pmu_statsfile /dev/null --quiet ${CMAKE_CURRENT_BINARY_DIR}/offsets -- ${CMAKE_CURRENT_BINARY_DIR}/kernel/include/asm_offsets.h
)

#
# Fixup the linker script template to match the target's memory map
# expectations.
#
execute_process(
        COMMAND sed -e "s/0xyyyyyyyy/${BOOTVM_LOADLINUX_ADDRESS}/" -e "s/0xxxxxxxxx/${BOOTVM_LOAD_ADDRESS}/" ${CMAKE_SOURCE_DIR}/stake/vapa_kernel.x.in
        OUTPUT_FILE  ${CMAKE_CURRENT_BINARY_DIR}/linux/vapa_kernel.x)

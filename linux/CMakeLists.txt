

file(
  GLOB
  LINUX_SRC
  "loadlinux.c"
  "loadlinux_event.S"
  "${CMAKE_SOURCE_DIR}/stake/bootvm_entry.S"
 )

add_executable(bootvm_loadlinux)
target_sources (bootvm_loadlinux PRIVATE ${LINUX_SRC})
add_dependencies (bootvm_loadlinux h2 h2kernel bootvm_image)

target_compile_options(bootvm_loadlinux
  PRIVATE
          "-DLINUX"
          "-DH2K_GUEST_START=0x02000000"
          "-DH2K_GUEST_END=${H2K_GUEST_END}"
          "-DH2K_BOOTVM_OFFSET=${BOOTVM_LOADLINUX_ADDRESS}"
          "-DLINUX_OFFSET_ADDR=0x00000000"
  )

target_link_options(bootvm_loadlinux
  PRIVATE
          "-mv${ARCHV}"
          "-moslib=h2"
          "-Wl,-L${CMAKE_BINARY_DIR}/kernel"
          "-Wl,-L${CMAKE_BINARY_DIR}/libs"
          "-Wl,--section-start=.entry=${BOOTVM_LOADLINUX_ADDRESS}"
          "-Wl,--script=${CMAKE_SOURCE_DIR}/stake/vapa_bootvm.x"
          "-Wl,--defsym=HEAP_SIZE=0x10000"
          "-Wl,--defsym=STACK_SIZE=0x1000"
          "-Wl,--defsym=linux_stext=${LINUX_LOAD_ADDRESS}"
)

add_custom_command(TARGET bootvm_loadlinux
  POST_BUILD
  COMMAND
  ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/bootvm_loadlinux ${CMAKE_CURRENT_BINARY_DIR}/bootvm_image.bin
)


target_include_directories(
  bootvm_loadlinux PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/../include
	${CMAKE_CURRENT_BINARY_DIR}/../kernel/include
  )

add_library(bootvm_linux)
add_dependencies (bootvm_linux bootvm_loadlinux)
target_sources(bootvm_linux
  PRIVATE
          ${CMAKE_SOURCE_DIR}/stake/bootvm.S
)
target_compile_options(bootvm_linux
  PRIVATE
          "-DH2K_GUEST_START=0x02000000"
          "-DH2K_GUEST_END=${H2K_GUEST_END}"
          "-DH2K_BOOTVM_OFFSET=${BOOTVM_LOADLINUX_ADDRESS}"
          "-DLINUX"
          "-DLINUX_OFFSET_ADDR=0x00000000"
  )

target_include_directories(
  bootvm_linux PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/../include
        ${CMAKE_CURRENT_BINARY_DIR}/../kernel/include
)


add_custom_command(TARGET bootvm_linux
    POST_BUILD
    COMMAND
        ${CMAKE_C_COMPILER}
          "-mv68"
          "-moslib=h2"
          "-Wl,-L${CMAKE_BINARY_DIR}/kernel"
          "-Wl,-L${CMAKE_BINARY_DIR}/libs"
          "-nostartfiles"
          "-Wl,--whole-archive"
          "../kernel/libh2kernel.a"
          "-Wl,-no-whole-archive"
          "-Wl,--section-start=.entry=${H2K_LINK_ADDR}"
          "-Wl,--section-start=.bootvm=${H2K_BOOT_VM}"
          "-Wl,--defsym=HEAP_SIZE=0x1"
          "-Wl,--defsym=STACK_SIZE=0x1"
          "-Wl,--defsym=H2K_ALLOC_HEAP_SIZE=0x20000"
          "-Wl,--defsym=H2K_KERNEL_NPAGES=0x1"
          "-Wl,--script=${CMAKE_BINARY_DIR}/linux/vapa_kernel.x"
          "-Wl,--defsym=__entry=${BOOTVM_LOAD_ADDRESS}"
          "-Wl,--entry=__entry"
          "-o" "loadlinux_tmp"
          "libbootvm_linux.a"

    COMMAND
          ${CMAKE_SOURCE_DIR}/stake/syms.pl loadlinux_tmp bootvm_loadlinux bootvm_image.sym

    COMMAND
        ${CMAKE_C_COMPILER}
          "-mv68"
          "-moslib=h2"
          "-Wl,-L${CMAKE_BINARY_DIR}/kernel"
          "-Wl,-L${CMAKE_BINARY_DIR}/libs"
          "-nostartfiles"
          "-Wl,--whole-archive"
          "../kernel/libh2kernel.a"
          "-Wl,-no-whole-archive"
          "-Wl,--section-start=.entry=${H2K_LINK_ADDR}"
          "-Wl,--section-start=.bootvm=${H2K_BOOT_VM}"
          "-Wl,--defsym=HEAP_SIZE=0x1"
          "-Wl,--defsym=STACK_SIZE=0x1"
          "-Wl,--defsym=H2K_ALLOC_HEAP_SIZE=0x20000"
          "-Wl,--defsym=H2K_KERNEL_NPAGES=0x1"
          "-Wl,--script=${CMAKE_BINARY_DIR}/linux/vapa_kernel.x"
          "-Wl,--defsym=__entry=${BOOTVM_LOAD_ADDRESS}"
          "-Wl,--entry=__entry"
          "-Wl,@bootvm_image.sym"
          "-o" "loadlinux"
          "libbootvm_linux.a"
)


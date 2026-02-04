

set(
  BOOTER_SRC
  "booter.c"
  "elf.c"
  "booter_event.S"
  "${CMAKE_SOURCE_DIR}/stake/bootvm_entry.S"
 )

add_executable(bootvm_image)
target_sources (bootvm_image PRIVATE ${BOOTER_SRC})
add_dependencies (bootvm_image h2 h2kernel)

target_compile_options(bootvm_image
  PRIVATE
          "-DH2K_GUEST_START=0x02000000"
          "-DH2K_GUEST_END=${H2K_GUEST_END}"
          "-DH2K_BOOTVM_OFFSET=${BOOTVM_H2_OFFSET}"
          "-DLINUX"
          "-DLINUX_OFFSET_ADDR=0x00000000"
          "-DFRAME_BUFFER=")

target_link_options(bootvm_image
  PRIVATE
          "-mv${ARCHV}"
          "-moslib=h2"
          "-Wl,-L${CMAKE_BINARY_DIR}/kernel"
          "-Wl,-L${CMAKE_BINARY_DIR}/libs"
          "-Wl,--section-start=.entry=${BOOTVM_H2_OFFSET}"
          "-Wl,--script=${CMAKE_SOURCE_DIR}/stake/vapa_bootvm.x"
          "-Wl,--defsym=HEAP_SIZE=0x1000000"
          "-Wl,--defsym=STACK_SIZE=0x1000"
          "-Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0"
)

add_custom_command(TARGET bootvm_image
  POST_BUILD
  COMMAND
  ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/bootvm_image ${CMAKE_CURRENT_BINARY_DIR}/bootvm_image.bin
)


target_include_directories(
  bootvm_image PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/../include
        ${CMAKE_CURRENT_BINARY_DIR}/../kernel/include
  )

add_library(bootvm)
add_dependencies (bootvm bootvm_image)
target_sources(bootvm
  PRIVATE 
          ${CMAKE_SOURCE_DIR}/stake/bootvm.S
)
target_compile_options(bootvm
  PRIVATE
          "-DH2K_GUEST_START=0x02000000"
          "-DH2K_GUEST_END=${H2K_GUEST_END}"
          "-DH2K_BOOTVM_OFFSET=${BOOTVM_H2_OFFSET}"
          "-DLINUX"
          "-DLINUX_OFFSET_ADDR=0x00000000"
          "-DFRAME_BUFFER=")

target_include_directories(
  bootvm PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/../include
        ${CMAKE_CURRENT_BINARY_DIR}/../kernel/include
)



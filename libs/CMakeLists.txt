

file(
  GLOB
  H2LIB_SRC
  "./h2_compat/alloc/*.[cS]"
  "./h2_compat/allsignal/*.[cS]"
  "./h2_compat/anysignal/*.[cS]"
  "./h2_compat/cache/*.[cS]"
  "./h2_compat/context/*.[cS]"
  "./h2_compat/debug/*.[cS]"
  "./h2_compat/error/*.[cS]"
  "./h2_compat/fastint/*.[cS]"
  "./h2_compat/kerror/*.[cS]"
  "./h2_compat/pipe/*.[cS]"
  "./h2_compat/printf/*.[cS]"
  "./h2_compat/prof/*.[cS]"
  "./h2_compat/signal/*.[cS]"
  "./h2_compat/sleep/*.[cS]"
  "./h2/config/*.[cS]"
  "./h2/cycles/*.[cS]"
  "./h2/fatal/*.[cS]"
  "./h2/futex/*.[cS]"
  "./h2/hwconfig/*.[cS]"
  "./h2/info/*.[cS]"
  "./h2/intpool/*.[cS]"
  "./h2/intwait/*.[cS]"
  "./h2/pmu/*.[cS]"
  "./h2/prefetch/*.[cS]"
  "./h2/prio/*.[cS]"
  "./h2/thread/*.[cS]"
  "./h2/tlb/*.[cS]"
  "./h2/vm/*.[cS]"
  "./h2/vmtraps/*.[cS]"
  "./h2/waitcycles/*.[cS]"
  "./posix/pthread/barrier/*.[cS]"
  "./posix/pthread/cond/*.[cS]"
  "./posix/pthread/misc/*.[cS]"
  "./posix/pthread/mutex/*.[cS]"
  "./posix/pthread/plainmutex/*.[cS]"
  "./posix/pthread/rwlock/*.[cS]"
  "./posix/pthread/sem/*.[cS]"
  "./posix/pthread/thread/*.[cS]"
  "./posix/pthread/tls/*.[cS]"
  "./syscall/angel/src/*.[cS]"
 )

add_library(h2 STATIC ${H2LIB_SRC})

target_compile_options(
  h2
  PRIVATE
          "-mhvx"
          "-mhvx-length=128B"
          "-DNULL_ANGEL_TRAP"
          "-DH2K_LINK_ADDR=0xff000000"
          "-DH2K_GUEST_END=0xfe000000"
          "-DH2K_GIT_COMMIT=0x585525d")

target_include_directories(
  h2 PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/../include
	${CMAKE_CURRENT_BINARY_DIR}/../kernel/include
  )


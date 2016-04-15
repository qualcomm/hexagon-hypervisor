

CC = hexagon-clang -mv$(TOOLARCH) -Iinclude -Wall -I../h2/include -I../posix/include -I../h2_compat/include 
AR = hexagon-ar

OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) -G0

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = $(wildcard src/*.S)
C_FILES = src/qurt_fatal.c src/qurt_memory.c src/qurtos_sclk.c src/qurtos_timer_libs.c src/qurtos_timer_server.c src/qurt_pgalloc.c src/qurt_thread.c src/qurt_timer.c src/qurt_timer_hw.c src/qurt_tls.c src/qurt_int.c src/qurt_fd.c src/qurt_qdi.c src/qurt_qdi_local_client.c src/qurt_qdi_signal.c src/qurt_mmap.c src/qurt_misc.c src/qurt_trace.c src/qmutex.c src/qurt_cache.c src/qurt_tlb.c src/qurt_s5alloc.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libqurt.a 

clean: 
	rm -f libqurt.a $(O_FILES)

libqurt.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


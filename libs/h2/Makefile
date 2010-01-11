
CC = qdsp6-gcc -mv$(ARCHV) -Iinclude -Wall
AR = qdsp6-ar

CFLAGS = -g -O2 

export H2_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/h2_futex.S src/h2_intwait.S src/h2_mutex.S src/h2_thread.S src/h2_cycles.S src/h2_sem.S src/h2_fastint.S src/h2_prio.S src/h2_allsignal.S src/h2_anysignal.S src/h2_prefetch.S src/h2_cond.S src/h2_barrier.S src/h2_config.S
C_FILES = src/h2_alloc.c src/h2_pipe.c src/h2_printf.c src/h2_rmutex.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libh2.a 

clean: 
	rm -f libh2.a $(O_FILES)

libh2.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


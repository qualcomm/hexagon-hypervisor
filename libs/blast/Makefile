
CC = qdsp6-gcc -mv3 -Iinclude -Wall
AR = qdsp6-ar

CFLAGS = -g -O2 

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/blast_futex.S src/blast_intwait.S src/blast_mutex.S src/blast_thread.S src/blast_cycles.S src/blast_sem.S src/blast_fastint.S src/blast_prio.S src/blast_allsignal.S src/blast_anysignal.S src/blast_prefetch.S src/blast_cond.S src/blast_barrier.S
C_FILES = src/blast_alloc.c src/blast_pipe.c src/blast_printf.c src/blast_rmutex.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libblast.a 

clean: 
	rm -f libblast.a $(O_FILES)

libblast.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


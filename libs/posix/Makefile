LIBNAME=posix

CC = qdsp6-gcc -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../blast/include
AR = qdsp6-ar

CFLAGS = -g -O2 -G0

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = 
#C_FILES = src/pthread.c src/pthread_internal.c src/signal.c src/semaphore.c src/mqueue.c src/mq_internal.c src/time.c
C_FILES = src/pthread.c src/pthread_internal.c src/semaphore.c src/mqueue.c src/mq_internal.c src/signal.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: lib${LIBNAME}.a

clean:
	rm -f libposix.a $(O_FILES)

lib${LIBNAME}.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


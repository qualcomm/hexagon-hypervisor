LIBNAME=posix

OPTIMIZE := -Os

CFLAGS =  -mv$(TOOLARCH) -Iinclude -Wall -I../h2/include -I../qurt/include -g $(OPTIMIZE) -G0
ASFLAGS = -mv$(TOOLARCH) -Iinclude -Wall -I../h2/include -I../qurt/include 

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = 
#C_FILES = src/pthread.c src/pthread_internal.c src/signal.c src/semaphore.c src/mqueue.c src/mq_internal.c src/time.c
C_FILES = src/pthread.c src/pthread_internal.c src/mqueue.c src/semaphore.c src/mq_internal.c src/signal.c src/time.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: lib${LIBNAME}.a

clean:
	rm -f libposix.a $(O_FILES)

lib${LIBNAME}.a: $(O_FILES)
	$(AR) cru $@ $(sort $(O_FILES))


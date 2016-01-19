

CC = hexagon-clang -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../qurt/include -D_PROVIDE_POSIX_TIME_DECLS=1
AR = hexagon-ar

OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) -G0

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = $(wildcard src/*.S)
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libposix.a 

clean: 
	rm -f libposix.a $(O_FILES)

libposix.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)

pthread_test: pthread_test.c
	hexagon-clang -mv5 -moslib=h2 -I../../install/include -L../../install/lib -Wl,--defsym='tdata_start=ADDR(".tdata")' -Wl,--defsym='tdata_end=ADDR(".tdata")+SIZEOF(".tdata")' -Wl,--defsym='tbss_start=ADDR(".tbss")' -Wl,--defsym='tbss_end=ADDR(".tbss")+SIZEOF(".tbss")' -Wl,--section-start=.start=0x01000000 -o pthread_test pthread_test.c

pthread_test_run: pthread_test
	hexagon-sim -- ../../install/bin/booter pthread_test


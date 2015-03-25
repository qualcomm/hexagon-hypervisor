

CC = qdsp6-gcc -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include
AR = qdsp6-ar

OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) -G0

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/qurt_lifo.S 
C_FILES = src/qurt_tls.c src/qurt_thread.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libqurt.a 

clean: 
	rm -f libqurt.a $(O_FILES)

libqurt.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)




CC = hexagon-clang -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../qurt/include
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


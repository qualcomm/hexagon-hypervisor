

CC = hexagon-clang -mv$(TOOLARCH) -Iinclude -Wall -Wno-builtin-requires-header -I../h2/include -I../posix/include -I../h2_compat/include
AR = hexagon-ar

OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) $(OPT_ADD) -G0

export QURT_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = $(wildcard src/*.S)
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libqurt.a 

clean: 
	rm -f libqurt.a src/*.o

libqurt.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


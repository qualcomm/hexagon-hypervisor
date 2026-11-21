



# C library selection (the Picolibc target and cstdlib) comes from Makefile.inc.libc.
include ../../scripts/Makefile.inc.libc
TARGET_FLAG = $(OSLIB_CFLAGS)

CC = hexagon-clang
AR = hexagon-ar
CFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include -g $(OPTIMIZE) $(OPT_ADD) -G0
ASFLAGS = $(TARGET_FLAG) -mv$(TOOLARCH) -DARCHV=$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include
OPTIMIZE := -O2

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = $(wildcard src/*.S)
C_FILES = $(wildcard src/*.c)

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libblast.a 

clean: 
	rm -f libblast.a src/*.o

libblast.a: $(O_FILES)
	rm -f $@
	$(AR) cru $@ $(sort $(O_FILES))




CC = hexagon-gcc
AR = hexagon-ar
CFLAGS = -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include -g $(OPTIMIZE) -G0
ASFLAGS = -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include
OPTIMIZE := -O2

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/blast_lifo.S 
C_FILES = src/blast_fd.c src/blast_int.c src/blast_tls.c src/blast_anysignal.c src/blast_thread.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libblast.a 

clean: 
	rm -f libblast.a $(O_FILES)

libblast.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


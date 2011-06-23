

CC = qdsp6-gcc -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include
AR = qdsp6-ar

OPTIMIZE := -O2

CFLAGS = -g $(OPTIMIZE) -G0

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/blast_lifo.S 
C_FILES = src/blast_fd.c src/blast_int.c src/blast_tls.c src/blast_anysignal.c src/blast_thread.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libblast.a 

clean: 
	rm -f libblast.a $(O_FILES)

libblast.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


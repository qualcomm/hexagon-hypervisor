
CC = qdsp6-gcc -mv$(ARCHV) -Iinclude -Wall -I../../h2/include
AR = qdsp6-ar

CFLAGS = -g -O2 

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/angel.S
C_FILES = src/allsyscalls.c src/tls.c src/mutex.c src/sbrk.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libangel.a 

clean: 
	rm -f libangel.a $(O_FILES)

libangel.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)



CC = qdsp6-gcc
AR = qdsp6-ar

OPTIMIZE := -O2

CFLAGS = -mv$(ARCHV) -Iinclude -Wall -I../../h2/include -g $(OPTIMIZE) -G0
ASFLAGS = -mv$(ARCHV) -Iinclude -Wall -I../../h2/include 

export BLAST_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = src/angel.S
C_FILES = src/allsyscalls.c src/tls.c src/mutex.c src/sbrk.c src/open.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libangel.a 

clean: 
	rm -f libangel.a $(O_FILES)

libangel.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


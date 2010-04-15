

CC = qdsp6-gcc -mv$(ARCHV) -Iinclude -Wall -I../h2/include -I../../kernel/include
AR = qdsp6-ar

CFLAGS = -g -O2 -G0

export QUBE_LIB_INCLUDE=$(shell pwd)/include

ASM_FILES = 
C_FILES = src/qmsgq.c

#src/qthread.c

O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

all: libqube.a 

clean: 
	rm -f libqube.a $(O_FILES)

libqube.a: $(O_FILES)
	$(AR) cru $@ $(O_FILES)


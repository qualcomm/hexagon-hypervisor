#Hey emacs this is a -*- Makefile -*-
BOOT=1
OBJS+=test.o
EXEC=test.elf

include Makefile.inc

CFLAGS += -v -G0

BOOTVM_PROG := booter

all: test.o 
	${CC} $(LDFLAGS) $(CFLAGS)  -Wl,--section-start=.start=$(H2K_GUEST_START) -o test test.o

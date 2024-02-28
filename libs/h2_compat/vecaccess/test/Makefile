#Hey emacs this is a -*- Makefile -*-
BOOT=1
OBJS+=test.o
EXEC=test.elf

include Makefile.inc

CFLAGS += -v -G0

BOOTVM_PROG := booter

all: test.o $(BOOTVM_PROG)
	${CC} $(LDFLAGS) $(CFLAGS)  -Wl,--section-start=.start=$(H2K_GUEST_START) -o hello hello.o

$(BOOTVM_IMAGE): booter.o elf.o booter_event.o $(BOOTVM_IMAGE_DEPS)
	${CC} $(LDFLAGS) $(CFLAGS) $(BOOTVM_IMAGE_CFLAGS) -Wl,--defsym=HEAP_SIZE=$(BOOTVM_IMAGE_HEAP_SIZE) -Wl,--defsym=STACK_SIZE=0x1000 -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0 -o $@ booter.o elf.o booter_event.o $(BOOTVM_ENTRY_O)
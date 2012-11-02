INC_KERNEL=1

OBJS+=main.o
EXEC=test.elf

include Makefile.inc

# big allocator heap for all those threads; size in words
LDFLAGS += -Wl,--defsym=H2K_ALLOC_HEAP_SIZE=0x20000

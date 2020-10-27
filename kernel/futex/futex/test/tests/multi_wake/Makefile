INC_KERNEL=1
BOOT=1
BOOTER_ARGS := --num_vcpus 512

OBJS+=main.o
EXEC=test.elf

include Makefile.inc

LDFLAGS += -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0

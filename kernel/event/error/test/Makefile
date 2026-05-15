
OBJS+=test.o test_asm.o
EXEC=test.elf

include Makefile.inc

KERNEL_BUILD_DIR := $(patsubst %/install,%/build/kernel,$(INSTALLPATH))
CFLAGS += -ffixed-r28 -I$(KERNEL_BUILD_DIR)/include
ASFLAGS += -ffixed-r28 -I$(KERNEL_BUILD_DIR)/include


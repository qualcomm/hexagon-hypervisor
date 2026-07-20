
OBJS+=test.o
EXEC=test.elf

include Makefile.inc

KERNEL_BUILD_DIR := $(INSTALLPATH)/../build/kernel
CFLAGS += -ffixed-r28 -I$(KERNEL_BUILD_DIR)/include

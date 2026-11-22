INC_KERNEL=1

OBJS+=test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/traps/hwconfig $(H2DIR)/libs/h2/hwconfig
RECOMPILE_SRCS_PREFS += hwconfig h2_hwconfig_imp

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc

KERNEL_BUILD_DIR := $(INSTALLPATH)/../build/kernel
CFLAGS += -ffixed-r28 -I$(KERNEL_BUILD_DIR)/include

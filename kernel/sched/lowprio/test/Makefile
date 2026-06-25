INC_KERNEL=1

OBJS+=test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/sched/lowprio
RECOMPILE_SRCS_PREFS += lowprio

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc

CFLAGS+= -ffixed-r28 -O0

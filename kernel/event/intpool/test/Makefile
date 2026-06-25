STANDALONE=1

OBJS+=test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/data/runlist
RECOMPILE_DIRS += $(H2DIR)/kernel/data/readylist
RECOMPILE_DIRS += $(H2DIR)/kernel/sched/lowprio
RECOMPILE_SRCS_PREFS += runlist
RECOMPILE_SRCS_PREFS += readylist
RECOMPILE_SRCS_PREFS += lowprio

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc

#  Empty out the OSLIB variable
OSLIB=

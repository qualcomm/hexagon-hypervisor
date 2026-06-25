STANDALONE=1

OBJS+=test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/data/runlist
RECOMPILE_DIRS += $(H2DIR)/kernel/data/readylist
RECOMPILE_DIRS += $(H2DIR)/kernel/sched/lowprio
RECOMPILE_DIRS += $(H2DIR)/kernel/mem/asid
RECOMPILE_SRCS_PREFS += runlist
RECOMPILE_SRCS_PREFS += readylist
RECOMPILE_SRCS_PREFS += lowprio
RECOMPILE_SRCS_PREFS += asid

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc
OSLIB=

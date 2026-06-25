STANDALONE=1

#  Wow.  make is totally buggy with this setup for some reason.
#  Resorting to old school.

#  Hrm...  check_sanity assertion stuff causing problems.  Fix later.

OBJS += harness.o
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

OSLIB=

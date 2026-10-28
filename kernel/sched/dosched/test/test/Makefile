STANDALONE=1

#  Wow.  make is totally buggy with this setup for some reason.
#  Resorting to old school.

#  Hrm...  check_sanity assertion stuff causing problems.  Fix later.

OBJS += harness.o
RECOMPILE_DIRS += $(H2DIR)/kernel/data/readylist
RECOMPILE_SRCS_PREFS += readylist

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc

OSLIB=

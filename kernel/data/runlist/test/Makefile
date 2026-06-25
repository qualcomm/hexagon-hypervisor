STANDALONE=1

OBJS+=test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/data/runlist
RECOMPILE_SRCS_PREFS += runlist

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc
OSLIB=

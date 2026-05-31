STANDALONE=1

OBJS += test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/time/timer
RECOMPILE_SRCS_PREFS += timer

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

include Makefile.inc
OSLIB=

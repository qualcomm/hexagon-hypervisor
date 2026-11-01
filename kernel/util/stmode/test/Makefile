
INC_KERNEL=1

OBJS+=test.o
RECOMPILE_DIRS += $(H2DIR)/kernel/traps/hwconfig $(H2DIR)/libs/h2/hwconfig
RECOMPILE_SRCS_PREFS += hwconfig h2_hwconfig_imp

EXTRA_CFLAGS += -DTESTING
EXTRA_ASFLAGS += -DTESTING

EXEC=test.elf

EXTRA_CFLAGS  += -ffixed-r28
EXTRA_ASFLAGS += -ffixed-r28

include Makefile.inc

# allow local override of H2K_thread_create_no_squash to inherit caller privilege
LDFLAGS += -Wl,--allow-multiple-definition

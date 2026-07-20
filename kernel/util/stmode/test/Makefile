
INC_KERNEL=1

OBJS+=test.o
EXEC=test.elf

EXTRA_CFLAGS  += -ffixed-r28
EXTRA_ASFLAGS += -ffixed-r28

include Makefile.inc

# allow local override of H2K_thread_create_no_squash to inherit caller privilege
LDFLAGS += -Wl,--allow-multiple-definition

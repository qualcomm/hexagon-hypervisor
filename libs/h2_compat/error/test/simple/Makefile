BOOT=1
INC_KERNEL=1

OBJS+=test.o
EXEC=test.elf

include Makefile.inc

BOOTER_ARGS := --expect_status 0xaaafa7a1
POST_RUN := grep 'DEBUG DUMP' test.log && perl -pi -e 's/VM 2 status 0xaaafa7a1/TEST PASSED/' test.log


BOOT=1

# Signed spin count fed to the guest as argv[1] for the suite run.
# Sign selects which racing thread is delayed (>=0 blocker, <0 exiter);
# 0 means no artificial delay. Override on the command line, e.g.
# make ... GUEST_ARGS=50000  or  make ... GUEST_ARGS=-50000
# 653 is the v81/opt tightest-race value: it delays the blocker just enough
# that A's teardown scan catches B still RUNNING, landing inside the window.
GUEST_ARGS=653

OBJS+=test.o
EXEC=test.elf

include Makefile.inc

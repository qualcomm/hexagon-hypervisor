INC_KERNEL=1
BOOT=1

OBJS+=test.o
EXEC=test.elf

# Optional knobs, e.g. VC_MT_CFLAGS='-DVC_MT_LINES=50' for a quick smoke run.
CFLAGS += $(VC_MT_CFLAGS)

# 8 threads x VC_MT_LINES synchronous console round-trips are slow in the sim;
# give this test more CPU budget than the default 600s so it is not SIGKILLed.
ULIMIT := 3600

# After the run, re-parse the full log: per-thread completeness + CRC integrity
# + (for virtio streams only) the booter-enabled-console check.  The validator
# appends TEST PASSED / FAIL: to test.log so the results.txt grep reports
# correctly.  Deferred '=' so $(PYTHON) resolves after Makefile.inc.test defines
# it.
POST_RUN = $(PYTHON) validate_log.py test.log

include Makefile.inc

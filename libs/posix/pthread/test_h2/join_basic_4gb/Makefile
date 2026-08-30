BOOT=1

OBJS+=test.o
EXEC=test.elf

include Makefile.inc

# Place the guest VM's physical base above 4GB, exercising booter's
# aperture/bounce-buffer ELF load path (booter_grow_guest_window() /
# booter_guest_ptr() in booter/booter.c) and LINEAR guest translation for a
# real multi-threaded guest, not just a load-time byte-correctness check.
# Manually verified passing (VM exit status 0x0) -- see notes.md "RESOLVED:
# residual join_basic failure was a self-inflicted repro artifact, not a
# bug" and "OK now try testing a VM loaded above 4GB".
BOOTER_ARGS := --guest_base 0x400000000

# NOT YET ADDED to any scripts/testlist.v* -- intentionally excluded from the
# regression for now (this is a template for a later phase, not an active
# test). Reason: above a 4GB guest_base, the guest's buffered stdio does not
# reliably flush before this test's own results.txt rule greps test.log for
# "TEST PASSED" (scripts/Makefile.inc.test) -- a separate, already-tracked,
# still-open limitation (the Angel/trap0 32-bit argument ceiling on the
# guest's own I/O path, one syscall-register-width ceiling further out than
# the booter-side one fixed for VM loading). Running this test today would
# most likely register as a false FAIL (empty results.txt) despite the VM
# itself exiting with status 0, exactly as seen when this was run manually.
#
# To activate later: add
#   ./libs/posix/pthread/test_h2/join_basic_4gb
# to scripts/testlist.v61 (resolved via symlink by v62/v65/v66t/v68/v73/v81),
# once either (a) the Angel guest-I/O ceiling above is fixed so guest stdio
# actually reaches the console, or (b) this test's pass/fail check is changed
# to key off the VM exit status (booter's "VM <n> status 0x0" line) instead
# of grepping guest stdout for "TEST PASSED".

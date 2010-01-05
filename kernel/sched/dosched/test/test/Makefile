STANDALONE=1

#  Wow.  make is totally buggy with this setup for some reason.
#  Resorting to old school.

#  Hrm...  check_sanity assertion stuff causing problems.  Fix later.

SRCS = h2/kernel/sched/dosched/dosched.ref.c \
	h2/kernel/sched/check_sanity/check_sanity.ref.c \
	h2/kernel/checkers/checker_kernel_locked/checker_kernel_locked.check.c \
	h2/kernel/checkers/checker_ring/checker_ring.check.c \
	h2/kernel/checkers/checker_ready/checker_ready.check.c \
	h2/kernel/checkers/checker_runlist/checker_runlist.check.c \
	h2/kernel/data/runlist/runlist.ref.c \
	h2/kernel/data/readylist/readylist.ref.c \
	h2/kernel/sched/lowprio/lowprio.ref.c \
	h2/kernel/util/ring/ring.ref.c \
	harness.c

EXEC=test.elf

include Makefile.inc

#  This will cause a warning.
${EXEC}:  ${SRCS} h2
	${CC} ${CFLAGS} -o $@ ${SRCS}

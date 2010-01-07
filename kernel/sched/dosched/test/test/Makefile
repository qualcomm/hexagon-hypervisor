STANDALONE=1

#  Wow.  make is totally buggy with this setup for some reason.
#  Resorting to old school.

#  Hrm...  check_sanity assertion stuff causing problems.  Fix later.

EXEC=test.elf

include Makefile.inc

SRCS = ${H2DIR}/kernel/sched/dosched/dosched.ref.c \
	${H2DIR}/kernel/sched/check_sanity/check_sanity.ref.c \
	${H2DIR}/kernel/checkers/checker_kernel_locked/checker_kernel_locked.check.c \
	${H2DIR}/kernel/checkers/checker_ring/checker_ring.check.c \
	${H2DIR}/kernel/checkers/checker_ready/checker_ready.check.c \
	${H2DIR}/kernel/checkers/checker_runlist/checker_runlist.check.c \
	${H2DIR}/kernel/data/runlist/runlist.ref.c \
	${H2DIR}/kernel/data/readylist/readylist.ref.c \
	${H2DIR}/kernel/sched/lowprio/lowprio.ref.c \
	${H2DIR}/kernel/util/ring/ring.ref.c \
	harness.c



#  This will cause a warning.
${EXEC}:  ${SRCS} 
	${CC} ${CFLAGS} -o $@ ${SRCS}

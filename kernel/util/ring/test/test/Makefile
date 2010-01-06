STANDALONE=1

#  Wow.  make is totally buggy with this setup for some reason.
#  Resorting to old school.

SRCS = h2/kernel/checkers/checker_ring/checker_ring.check.c \
	h2/kernel/checkers/checker_ready/checker_ready.check.c \
	h2/kernel/checkers/checker_runlist/checker_runlist.check.c \
	h2/kernel/data/runlist/runlist.ref.c \
	h2/kernel/data/readylist/readylist.ref.c \
	h2/kernel/sched/lowprio/lowprio.ref.c \
	h2/kernel/util/ring/ring.ref.c \
	harness.c ring_append.c ring_insert.c ring_remove_append.c ring_remove.c


CFLAGS += -I.
EXEC=test.elf

include Makefile.inc
${SRCS}: h2

#  This will cause a warning.
${EXEC}:  ${SRCS} 
	${CC} ${CFLAGS} -o $@ ${SRCS}


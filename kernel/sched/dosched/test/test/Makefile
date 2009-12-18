
default: runtests

SRCS = ../../dosched.ref.c \
	../../../../checkers/checker_ring/checker_ring.check.c \
	../../../../checkers/checker_ready/checker_ready.check.c \
	../../../../checkers/checker_runlist/checker_runlist.check.c \
	../../../../data/runlist/runlist.ref.c \
	../../../../data/readylist/readylist.ref.c \
	../../../../sched/lowprio/lowprio.ref.c \
	../../../../util/ring/ring.ref.c \
	harness.c

cov: test
	qdsp6-sim --profile test
	qdsp6-gmon-merge -o gmon.out gmon.t_*
	qdsp6-cov -i test -o test.cov.out -c test.cov.csv gmon.out 

test: $(SRCS)
	qdsp6-gcc -mv3 -O -DARCHV=3 -Wall -I../../../../include -I. -o test $(SRCS)

runtests: test
	qdsp6-sim test

clean:
	rm -f test gmon.out gmon.t* *.cov.* stats.txt


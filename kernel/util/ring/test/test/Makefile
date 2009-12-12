
default: runtests

SRCS = ../../ring.ref.c ../../../../checkers/checker_ring/checker_ring.check.c *.c

test:
	qdsp6-gcc -mv3 -DARCHV=3 -I../../../../include -I. -o test $(SRCS)

runtests: test
	qdsp6-sim test


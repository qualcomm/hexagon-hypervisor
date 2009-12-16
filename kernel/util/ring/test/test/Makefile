
default: runtests

SRCS = ../../ring.ref.c ../../../../checkers/checker_ring/checker_ring.check.c *.c

cov: test
	qdsp6-sim --profile test
	qdsp6-gmon-merge -o gmon.out gmon.t_*
	qdsp6-cov -i test -o test.cov.out -c test.cov.csv gmon.out 

test: $(SRCS)
	qdsp6-gcc -mv3 -DARCHV=3 -I../../../../include -I. -o test $(SRCS)

runtests: test
	qdsp6-sim test

clean:
	rm -f test gmon.out gmon.t* *.cov.* stats.txt


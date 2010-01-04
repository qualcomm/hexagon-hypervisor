
default: runtests

SRCS = test.c test_asm.S

cov: test
	qdsp6-sim --profile test
	qdsp6-gmon-merge -o gmon.out gmon.t_*
	qdsp6-cov -i test -o test.cov.out -c test.cov.csv gmon.out 

test: $(SRCS)
	qdsp6-gcc -mv3 -O -DARCHV=3 -Wall -I../../../include -I. -L../../../build/ref -o test $(SRCS) -lh2kernel

runtests: test
	qdsp6-sim test

clean:
	rm -f test gmon.out gmon.t* *.cov.* stats.txt


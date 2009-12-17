
################################################################################
#  Test makefile doesn't share above the test dir
################################################################################

UNITDIR=${CURDIR}/${UPDIR}..

#  symbolic link to the unit
unit:
	-ln -s ${UNITDIR} $@

#  symbolic link to h2 project directory
h2: unit
	-ln -s unit/../../.. $@

################################################################################
#  Tool stuff
################################################################################

ARCH=qdsp6
SIM=${ARCH}-sim
CC=${ARCH}-gcc
AS=${ARCH}-as
LD=${ARCH}-ld
AR=${ARCH}-ar
OBJDUMP=${ARCH}-objdump
GMON_MERGE=${ARCH}-gmon-merge
COV=${ARCH}-cov

################################################################################
#  Rules for building the kernel if necessary; should be built way before any
#  batch jobs are run else they will maul each other in the animal farm.
#
#  Actually, I've broken the dependency between the libs and the executable so 
#  they will never actually try to build the libs; if you need the libs, you 
#  should 'make libs' before you run.
################################################################################

LIBH2_DIR=h2/libs
LIBH2=${LIBH2_DIR}/libh2.a

LIBH2KERNEL_DIR=h2/kernel
LIBH2KERNEL=${LIBH2KERNEL_DIR}/libh2kernel.a
LIBH2CHECK=${LIBH2KERNEL_DIR}/libh2check.a

#  Needs to have the tools already in path...
#SIMPATH=/pkg/qct/software/hexagon/releases/tools/1.2/qc/bin
#TOOLPATH=/pkg/qct/software/hexagon/releases/tools/1.2/gnu/bin
#COVPATH=/pkg/qct/software/hexagon/releases/tools/1.2/examples/utils/bin

${LIBH2KERNEL}:
	cd ${LIBH2KERNEL_DIR};\
	make ref DEBUG=1;\
	cp build/ref/libh2kernel.a .

${LIBH2CHECK}:
	cd ${LIBH2KERNEL_DIR};\
	make check DEBUG=1;\
	cp build/ref/libh2check.a .

${LIBH2}:
	cd ${LIBH2_DIR};\
	make DEBUG=1

libs: ${LIBH2} ${LIBH2CHECK} ${LIBH2KERNEL}

################################################################################
#  Standard compilation flags
################################################################################

#  Turn on call() support for assertion checking
CFLAGS += -DDEBUG
CFLAGS += -I h2/kernel/include -moslib=h2 -mv3 -DARCHV=3 -Wall -I h2/libs/h2/include
LDFLAGS+=-moslib=h2 -lh2kernel -lh2 -lh2check -L h2/kernel -L h2/libs -mv3


################################################################################
#  Convenience shortcuts
################################################################################

ifdef SUBDIRS

################################################################################
#  Recursive subdirectory traversal
################################################################################

SUBDIR_MAKE=for SUBDIR in ${SUBDIRS}; do\
	$(MAKE) -C $$SUBDIR $(MAKECMDGOALS);\
	done

SUBDIR_TARGETS=clean all

clean all:
	${SUBDIR_MAKE}

#  The plan was to do this recursive gmon/coverage thing, but that doesn't
#  work so well with multiple binaries

else

################################################################################
#  These are for actual terminal directories
################################################################################

%.objdump: %.elf
	${OBJDUMP} -D ${@:.objdump=.elf} > $@

${EXEC}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}

#  Todo:  need a watchdog to kill sims after some time; possibly use the "forked killer"
test.log: ${EXEC}
	${SIM} ${EXEC} --profile | tee $@

# Coverage stuff -- single testcase

gmon: test.log
	${GMON_MERGE} -o gmon gmon.t_*

COVFILES=cov.txt cov.csv
GMON_FILES=gmon gmon.t_0 gmon.t_1 gmon.t_2 gmon.t_3 gmon.t_4 gmon.t_5

${COVFILES}: ${EXEC} gmon
	${COV} -i ${EXEC} -o cov.txt -c cov.csv gmon

CLEANUP=${OBJS} ${EXEC} ${COVFILES} ${GMON_FILES} test.log results.txt

clean:
	rm -rf ${CLEANUP}

#  Pretty much a binary yes/no if the test passed or failed.
results.txt: test.log
	grep "TEST PASSED" test.log > $@

all:  results.txt ${COVFILES}


endif

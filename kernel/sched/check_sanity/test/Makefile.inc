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

#SIMPATH=/pkg/qct/software/hexagon/releases/tools/1.2/qc/bin
#TOOLPATH=/pkg/qct/software/hexagon/releases/tools/1.2/gnu/bin

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

################################################################################
#  Rules for building the kernel if necessary; should be built way before any
#  batch jobs are run else they will maul each other in the farm.
################################################################################

LIBH2_DIR=h2/libs
LIBH2=${LIBH2_DIR}/libh2.a

LIBH2KERNEL_DIR=h2/kernel
LIBH2KERNEL=${LIBH2KERNEL_DIR}/libh2kernel.a

#  Needs to have the tools already in path...
#  "make ref" should be "make ref DEBUG=1", add when kernel_locked is in there
${LIBH2KERNEL}: h2
	cd ${LIBH2KERNEL_DIR};\
	make ref;\
	cp build/ref/libh2kernel.a .

${LIBH2}: h2
	cd ${LIBH2_DIR};\
	make;\
	cp libh2.a ../kernel

libs:  ${LIBH2} ${LIBH2KERNEL}

################################################################################
#  Standard compilation flags
################################################################################

#  Turn on call() support for assertion checking
#CFLAGS += -DDEBUG
CFLAGS += -I h2/kernel/include -moslib=h2 -mv3 -DARCHV=3
LDFLAGS+=-moslib=h2 -lh2kernel -lh2 -L h2/kernel -mv3


################################################################################
#  Convenience shortcuts
################################################################################

%.objdump: %.elf
	${OBJDUMP} -D ${@:.objdump=.elf} > $@


${EXEC}: ${OBJS} h2
	${CC} -o $@ ${OBJS} ${LDFLAGS}

#  Todo:  need a watchdog to kill sims after some time; possibly use the "forked killer"
test.log: ${EXEC}
	${SIM} ${EXEC} > $@

clean:
	rm -rf ${CLEANUP}

#  Pretty much a binary yes/no if the test passed or failed.
results.txt: test.log
	grep "TEST PASSED" test.log > $@

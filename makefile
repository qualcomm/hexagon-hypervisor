
SIZE_TOOL = /prj/dsp/qdsp6/arch/scripts/section_size.py
LD = qdsp6-gcc

COM_CFLAGS=
COM_LDFLAGS=

#If you are running outside hexframe.. sensible defaults
ifeq (,${BUILD_DIR})
ARCHV=4
CC=qdsp6-gcc
RUN=qdsp6-sim
SIMF=--timing
#hexframe likes duplication ATM Machine, PIN Number, Q6VERSION Version....
else
ARCHV=$(subst v,,$(Q6VERSION))
endif

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(PWD)/install
endif

clean:
	make -C kernel ARCHV=$(ARCHV) clean && \
	make -C libs ARCHV=$(ARCHV) clean && \
	make -f scripts/Makefile.coverage clean && \
	make -f scripts/Makefile.coverage clean_top && \
	make -f scripts/docs/Makefile.sphinx clean && \
	rm -Rf size test.exe stats.txt install kernel/stats.txt

opt: clean
	make -C kernel ARCHV=$(ARCHV) opt_install && \
	make -C libs ARCHV=$(ARCHV) install

ref: clean
	make -C kernel ARCHV=$(ARCHV) ref_install && \
	make -C libs ARCHV=$(ARCHV) install

sim: ref
	$(CC) -mv$(ARCHV) -moslib=h2 -moslib=h2kernel -I$(INSTALLPATH)/include -L$(INSTALLPATH)/lib tst/test.c -o test.exe && \
	$(RUN) $(SIMF) -- test.exe;

size:
	qdsp6-objdump -h $(INSTALLPATH)/lib/*.a | $(SIZE_TOOL) text > size && \
	cat size;

cov:
	make -f scripts/Makefile.coverage prepare && \
	make -f scripts/Makefile.coverage all && \
	make -f scripts/Makefile.coverage cov.txt && \
	make -f scripts/Makefile.coverage report.html

doc:
	make -f scripts/docs/Makefile.sphinx prepare && \
	make -f scripts/docs/Makefile.sphinx doctest html

compat:
	cd install/lib ; ln -s libh2kernel.a libblastkernel.a ; ln -s libh2.a libblast.a


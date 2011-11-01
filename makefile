SHELL=/bin/bash
SIZE_TOOL = /prj/dsp/qdsp6/arch/scripts/section_size.py
LD = qdsp6-gcc

COM_CFLAGS=
COM_LDFLAGS=

#If you are running outside hexframe.. sensible defaults
ifeq (,${BUILD_DIR})
ifneq (,${Q6VERSION})
ARCHV=$(subst v,,$(Q6VERSION))
else
ARCHV=4
endif
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

OPT_JFLAG=-j 3
REF_JFLAG=-j 3
TEST_JFLAG=-j 8

ifeq ($(H2K_LINK_ADDR),)
export H2K_LINK_ADDR = 0xff000000
endif

ifeq ($(H2K_HEAP_SIZE),)
H2K_HEAP_SIZE = 0x100000
endif

all: ref doc gtags

distclean: clean docclean

clean: covclean ucosclean
	$(MAKE) -C kernel ARCHV=$(ARCHV) clean && \
	$(MAKE) -C libs ARCHV=$(ARCHV) clean && \
	rm -Rf size test.exe stats.txt install kernel/stats.txt

docclean:
	$(MAKE) -f scripts/docs/Makefile.sphinx clean

testclean covclean:
	$(MAKE) -f scripts/Makefile.coverage clean && \
	$(MAKE) -f scripts/Makefile.coverage clean_top

ucosclean:
	$(MAKE) -C ucos clean

opt:
	$(MAKE) $(OPT_JFLAG) -C kernel ARCHV=$(ARCHV) opt_install && \
	$(MAKE) $(OPT_JFLAG) -C libs ARCHV=$(ARCHV) install IMPL=opt && \
	$(MAKE) $(OPT_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;

ref:
	$(MAKE) $(REF_JFLAG) -C kernel ARCHV=$(ARCHV) ref_install && \
	$(MAKE) $(REF_JFLAG) -C libs ARCHV=$(ARCHV) install IMPL=ref && \
	$(MAKE) $(REF_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;

sim: ref
	$(CC) -mv$(ARCHV) -moslib=h2 -moslib=h2kernel -I$(INSTALLPATH)/include -L$(INSTALLPATH)/lib tst/test.c -o test.exe && \
	$(RUN) $(SIMF) -- test.exe;

size:
	qdsp6-objdump -h $(INSTALLPATH)/lib/*.a | $(SIZE_TOOL) text > size && \
	cat size;

t:
	/prj/dsp/qdsp6/arch/scripts/test_h2.pl $(TEST_H2_OPTS)

test: 
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare; \
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) tst; \
	cd ucos; $(MAKE) sim; cd ..; \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) report.html

cov:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare; \
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) all; \
	cd ucos; $(MAKE) sim; cd ..; \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) cov.txt; \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) report.html

test-check cov-check:
	$(MAKE) -f scripts/Makefile.coverage check && \
	cd ucos; $(MAKE) check && cd ..

doc:
	$(MAKE) -f scripts/docs/Makefile.sphinx prepare && \
	$(MAKE) -f scripts/docs/Makefile.sphinx doctest html

compat:
	cd install/lib ; ln -s libh2kernel.a libblastkernel.a ; ln -s libh2.a libblast.a

gtags:
	find kernel libs tst guest ucos linux -type f -print | gtags -w -v -f -
	htags -afhnosTxv --show-position

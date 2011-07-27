
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
	$(MAKE) -C kernel ARCHV=$(ARCHV) opt_install && \
	$(MAKE) -C libs ARCHV=$(ARCHV) install && \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;

ref:
	$(MAKE) -j 3 -C kernel ARCHV=$(ARCHV) ref_install && \
	$(MAKE) -j 3 -C libs ARCHV=$(ARCHV) install && \
	$(MAKE) -j 3 -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;

sim: ref
	$(CC) -mv$(ARCHV) -moslib=h2 -moslib=h2kernel -I$(INSTALLPATH)/include -L$(INSTALLPATH)/lib tst/test.c -o test.exe && \
	$(RUN) $(SIMF) -- test.exe;

size:
	qdsp6-objdump -h $(INSTALLPATH)/lib/*.a | $(SIZE_TOOL) text > size && \
	cat size;

test: 
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare; \
	$(MAKE) -j 8 -f scripts/Makefile.coverage ARCHV=$(ARCHV) tst; \
	cd ucos; $(MAKE) sim && cd .. && \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) report.html

cov:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare; \
	$(MAKE) -j 8 -f scripts/Makefile.coverage ARCHV=$(ARCHV) all; \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) cov.txt; \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) report.html

test-check cov-check:
	$(MAKE) -f scripts/Makefile.coverage check

doc:
	$(MAKE) -f scripts/docs/Makefile.sphinx prepare && \
	$(MAKE) -f scripts/docs/Makefile.sphinx doctest html

compat:
	cd install/lib ; ln -s libh2kernel.a libblastkernel.a ; ln -s libh2.a libblast.a

gtags:
	find kernel libs tst guest ucos -type f -print | gtags -w -v -f -
	htags -afhnosTxv --show-position

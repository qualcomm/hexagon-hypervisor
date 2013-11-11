
ifeq ($(TARGET), 8960)
ARCHV := 4
H2K_KERNEL_PGSIZE := 3
H2K_ALLOC_HEAP_SIZE := 0xb000
export H2K_EXTRA_CFLAGS += -DCOUNT_TLB_MISSES
endif

ifeq ($(TARGET), 8974)
ARCHV := 5
H2K_KERNEL_PGSIZE := 3
H2K_ALLOC_HEAP_SIZE := 0xb000
export H2K_EXTRA_CFLAGS += -DCOUNT_TLB_MISSES
endif


include scripts/Makefile.inc.tools

ifeq ($(INSTALLPATH),)
export INSTALLPATH := $(CURDIR)/install
endif

OPT_JFLAG=-j 3
REF_JFLAG=-j 3
TEST_JFLAG=-j 8

include scripts/Makefile.inc.config

all: ref doc gtags

distclean: clean docclean gtagsclean

clean: covclean ucosclean booterclean docclean
	$(MAKE) -C kernel ARCHV=$(ARCHV) clean
	$(MAKE) -C stake ARCHV=$(ARCHV) clean
	$(MAKE) -C libs ARCHV=$(ARCHV) clean
	rm -Rf size test.exe stats.txt install kernel/stats.txt

booterclean:
	$(MAKE) -C booter clean

docclean:
	$(MAKE) -C libs/docs/dox clean
	$(MAKE) -f scripts/docs/Makefile.sphinx clean

testclean covclean: ucosclean
	$(MAKE) -f scripts/Makefile.coverage clean && \
	$(MAKE) -f scripts/Makefile.coverage clean_top

ucosclean:
	$(MAKE) -C ucos clean

opt:
	$(MAKE) $(OPT_JFLAG) -C kernel ARCHV=$(ARCHV) opt_install && \
	$(MAKE) $(OPT_JFLAG) -C libs ARCHV=$(ARCHV) install IMPL=opt && \
	$(MAKE) $(OPT_JFLAG) -C stake ARCHV=$(ARCHV) install
	$(MAKE) $(OPT_JFLAG) -C booter ARCHV=$(ARCHV) install
	cp scripts/Makefile.inc.config $(INSTALLPATH)/scripts
	$(MAKE) $(OPT_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;
	echo "v$(ARCHV) $@" > $(INSTALLPATH)/ver

ref:
	$(MAKE) $(REF_JFLAG) -C kernel ARCHV=$(ARCHV) ref_install && \
	$(MAKE) $(REF_JFLAG) -C libs ARCHV=$(ARCHV) install IMPL=ref && \
	$(MAKE) $(REF_JFLAG) -C stake ARCHV=$(ARCHV) install
	$(MAKE) $(REF_JFLAG) -C booter ARCHV=$(ARCHV) install
	cp scripts/Makefile.inc.config $(INSTALLPATH)/scripts
	$(MAKE) $(REF_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;
	echo "v$(ARCHV) $@" > $(INSTALLPATH)/ver

sim: ref
	$(CC) -mv$(TOOLARCH) -moslib=h2 -moslib=h2kernel -I$(INSTALLPATH)/include -L$(INSTALLPATH)/lib tst/test.c -o test.exe && \
	$(RUN) $(SIMF) -- test.exe;

size:
	hexagon-objdump -h $(INSTALLPATH)/lib/*.a | $(SIZE_TOOL) text > size && \
	cat size;

t:
	/prj/dsp/qdsp6/arch/scripts/test_h2.pl $(TEST_H2_OPTS)

test: ucosclean
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) tst 2>&1 | tee test.out
	$(MAKE) -C ucos sim 2>&1 | tee make.log | tee -a test.out
	[ `fgrep -c -i warning: test.out` -eq 0 ]
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) report.html

cov:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) all
	$(MAKE) -C ucos sim 2>&1 | tee make.log
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) cov.rpt
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) report.html

.PHONY: check-fail test-check cov-check cov_fns

check-fail test-check cov-check:
	$(MAKE) -f scripts/Makefile.coverage check-fail
	$(MAKE) -C ucos check

check:
	$(MAKE) -f scripts/Makefile.coverage check
	$(MAKE) -C ucos check

doc:
	$(MAKE) -C libs/docs/dox
	$(MAKE) -f scripts/docs/Makefile.sphinx prepare
	$(MAKE) -f scripts/docs/Makefile.sphinx doctest html

compat:
	cd install/lib ; ln -s libh2kernel.a libblastkernel.a ; ln -s libh2.a libblast.a

.PHONY: gtags gtagsclean

gtags:
	find booter examples kernel libs linux perf scripts stake tst ucos -path kernel/include -prune -o -path libs/h2/include -prune -o -type f -print | gtags -I -w -v -f -
	htags -afhnosTxv --show-position

gtagsclean:
	rm -rf GPATH GRTAGS GSYMS GTAGS ID HTML

cov_fns:
	$(MAKE) clean ref ARCHV=v4 OPTIMIZE='-Os -fno-inline';
	./scripts/gen_cov_fns.pl > ./scripts/v4ref_cov_fns;
	$(MAKE) clean opt ARCHV=v4 OPTIMIZE='-Os -fno-inline';
	./scripts/gen_cov_fns.pl > ./scripts/v4opt_cov_fns;
	$(MAKE) clean ref ARCHV=v5 OPTIMIZE='-Os -fno-inline';
	./scripts/gen_cov_fns.pl > ./scripts/v5ref_cov_fns;
	$(MAKE) clean opt ARCHV=v5 OPTIMIZE='-Os -fno-inline';
	./scripts/gen_cov_fns.pl > ./scripts/v5opt_cov_fns;
	$(MAKE) clean ref ARCHV=v60 OPTIMIZE='-Os -fno-inline';
	./scripts/gen_cov_fns.pl > ./scripts/v60ref_cov_fns;
	$(MAKE) clean opt ARCHV=v60 OPTIMIZE='-Os -fno-inline';
	./scripts/gen_cov_fns.pl > ./scripts/v60opt_cov_fns;



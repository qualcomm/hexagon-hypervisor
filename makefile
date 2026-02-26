include scripts/Makefile.inc.config
include scripts/Makefile.inc.opensource
include scripts/Makefile.inc.version
include scripts/Makefile.inc.tools

ARCHV_LIST ?= 65 68 73 81
TESTOUT ?= test.out

TARGET ?= opt
ARCHV ?= 81

JFLAG ?= -j
OPT_JFLAG := $(JFLAG)
REF_JFLAG := $(JFLAG)
TEST_JFLAG ?= -j 8

ifeq ($(TARGET), 8960)
T := opt
ARCHV := 4
H2K_KERNEL_PGSIZE ?= 3
H2K_ALLOC_HEAP_SIZE ?= 0xb000
export H2K_EXTRA_CFLAGS += -DCOUNT_TLB_EVENTS
endif

ifeq ($(TARGET), 8974)
T := opt
ARCHV := 5
H2K_KERNEL_PGSIZE ?= 3
H2K_ALLOC_HEAP_SIZE ?= 0xb000
export H2K_EXTRA_CFLAGS += -DCOUNT_TLB_EVENTS
export USE_TCM ?= 1
endif

ifeq ($(TARGET), zebu_v60)
T := opt
ARCHV := 60
H2K_KERNEL_PGSIZE ?= 3
H2K_ALLOC_HEAP_SIZE ?= 0xb000
export H2K_EXTRA_CFLAGS += -DCOUNT_TLB_EVENTS
#export USE_TCM ?= 1
endif

ifeq ($(TARGET), zebu_v65)
T := opt
ARCHV := 65
H2K_KERNEL_PGSIZE ?= 3
H2K_ALLOC_HEAP_SIZE ?= 0xb000
export H2K_EXTRA_CFLAGS += -DCOUNT_TLB_EVENTS
#export USE_TCM ?= 1
endif

ifeq ($(TARGET), opt)
T := opt
#OPT_JFLAG :=
endif

ifeq ($(TARGET), opt_cov)
T := opt
OPT_JFLAG :=
export OPT_ADD := $(OPTIMIZE_COV)
endif

ifeq ($(TARGET), llvm_cov)
T := opt
OPT_JFLAG :=
export EN_LLVM_COV := 1
endif

ifeq ($(TARGET), opt_snap)
T := opt
#OPT_JFLAG :=
export H2K_LOAD_ADDR=0x00400000 
export H2K_EXTRA_CFLAGS+=-DNMI_STOP
endif

ifeq ($(TARGET), opt_tiny_snap)
override ARCHV := 65  # because some things call make with ARCHV=66t
T := opt
#OPT_JFLAG :=
export TINY_CORE=1
export H2K_LOAD_ADDR=0x00400000
export H2K_EXTRA_CFLAGS+=-DNMI_STOP
endif

ifeq ($(TARGET), ref)
T := ref
#REF_JFLAG :=
endif

ifeq ($(TARGET), ref_cov)
T := ref
REF_JFLAG :=
export OPT_ADD := $(OPTIMIZE_COV)
endif

ifeq ($(TARGET), debug)
T := ref
REF_JFLAG :=
export OPTIMIZE := $(OPTIMIZE_DEBUG)
endif

ifeq ($(TARGET), opt_si)
T := opt
#OPT_JFLAG :=
export H2K_LOAD_ADDR=0x84c00000
export H2K_GUEST_START=0x87000000
export NULL_ANGEL_TRAP=1
export MAGIC_ANGEL=--magic_angel
endif

# FIXME: Remove when cluster sched ported to opt
export OMIT_OPT=dosched resched


.PHONY: all
all:
	$(MAKE) $(JFLAG) $(T)

distclean: clean docclean gtagsclean

clean: covclean booterclean docclean qurtclean # ucosclean 
	$(MAKE) -C kernel ARCHV=$(ARCHV) clean
	$(MAKE) -C stake ARCHV=$(ARCHV) clean
	$(MAKE) -C libs ARCHV=$(ARCHV) clean
	rm -Rf size test.exe stats.txt install kernel/stats.txt

booterclean:
	$(MAKE) -C booter clean

docclean:
	$(MAKE) -C libs/docs/dox clean
	$(MAKE) -f scripts/docs/Makefile.sphinx clean

testclean covclean: qurtclean # ucosclean
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) clean && \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) clean_top

# ucosclean:
# 	$(MAKE) -C ucos clean

qurtclean:
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) clean
	$(MAKE) -f scripts/Makefile.qurt clean_top

opt:
ifeq ($(USE_PKW),1)
	@echo PKW_VERSIONS $(PKW_VERSIONS)
	pkw --which $(CC)
endif
	$(MAKE) $(OPT_JFLAG) -C kernel ARCHV=$(ARCHV) opt_install && \
	$(MAKE) $(OPT_JFLAG) -C libs ARCHV=$(ARCHV) install IMPL=opt && \
	$(MAKE) $(OPT_JFLAG) -C stake ARCHV=$(ARCHV) install
	$(MAKE) $(OPT_JFLAG) -C booter ARCHV=$(ARCHV) install
	cp scripts/Makefile.inc.config $(INSTALLPATH)/scripts
	cp scripts/Makefile.inc.opensource $(INSTALLPATH)/scripts
	cp scripts/devsim_v*.cfg $(INSTALLPATH)/scripts
	$(MAKE) $(OPT_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;
	echo "v$(ARCHV) $@ ${MAKEFLAGS}" > $(INSTALLPATH)/ver
	echo "sha_short $(H2K_GIT_COMMIT)" >> $(INSTALLPATH)/ver
	echo "sha_long $(H2K_GIT_COMMIT_LONG)" >> $(INSTALLPATH)/ver

ref:
ifeq ($(USE_PKW),1)
	@echo PKW_VERSIONS $(PKW_VERSIONS)
	pkw --which $(CC)
endif
	$(MAKE) $(REF_JFLAG) -C kernel ARCHV=$(ARCHV) ref_install && \
	$(MAKE) $(REF_JFLAG) -C libs ARCHV=$(ARCHV) install IMPL=ref && \
	$(MAKE) $(REF_JFLAG) -C stake ARCHV=$(ARCHV) install
	$(MAKE) $(REF_JFLAG) -C booter ARCHV=$(ARCHV) install
	cp scripts/Makefile.inc.config $(INSTALLPATH)/scripts
	cp scripts/Makefile.inc.opensource $(INSTALLPATH)/scripts
	cp scripts/devsim_v*.cfg $(INSTALLPATH)/scripts
	$(MAKE) $(REF_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;
	echo "v$(ARCHV) $@ ${MAKEFLAGS}" > $(INSTALLPATH)/ver
	echo "sha_short $(H2K_GIT_COMMIT)" >> $(INSTALLPATH)/ver
	echo "sha_long $(H2K_GIT_COMMIT_LONG)" >> $(INSTALLPATH)/ver

sim: ref
	$(CC) -mv$(TOOLARCH) -moslib=h2 -moslib=h2kernel -I$(INSTALLPATH)/include -L$(INSTALLPATH)/lib tst/test.c -o test.exe && \
	$(RUN) $(SIMF) -- test.exe;

size:
	hexagon-objdump -h $(INSTALLPATH)/lib/*.a | $(SIZE_TOOL) text > size && \
	cat size;

# t:
# 	/prj/dsp/qdsp6/arch/scripts/test_h2.pl $(TEST_H2_OPTS)

.PHONY: testall testall_prepare
testall: testall_prepare $(ARCHV_LIST)

testall_prepare:
	git clean -dxf -e 'jenkins[0-9]*'

.PHONY: $(ARCHV_LIST)
$(ARCHV_LIST):
	@echo '/// $@ opt ///' | tee -a $(TESTOUT)
	$(MAKE) ARCHV=$@ TARGET=opt all test
	git clean -dxf -e $(TESTOUT) -e 'jenkins[0-9]*'
	@echo '/// $@ ref ///' | tee -a $(TESTOUT)
	$(MAKE) ARCHV=$@ TARGET=ref all test
	git clean -dxf -e $(TESTOUT)  -e 'jenkins[0-9]*'

test:	h2_test check-fail

h2_test: # ucosclean
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) tst 2>&1 | tee -a $(TESTOUT)
#$(MAKE) -C ucos sim 2>&1 | tee make.log | tee -a $(TESTOUT)
	[ `fgrep -v "WARNING: Overriding currently set revid" $(TESTOUT) | fgrep -c -i warning:` -eq 0 ]
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) h2_report.html
	head -n -1 h2_report.html > report.html

qurt_test: ./qurt/test/testcases
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.qurt ARCHV=$(ARCHV) tst 2>&1 | tee $(TESTOUT)
#	[ `fgrep -c -i warning: $(TESTOUT)` -eq 0 ]
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) qurt_report.html

qurt_test_single: ./qurt/test/testcases
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.qurt ARCHV=$(ARCHV) TEST=$(TEST) tst_single 2>&1 | tee $(TESTOUT)
	[ `fgrep -c -i warning: $(TESTOUT)` -eq 0 ]

qurt_test_libs:
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) qurt_test_libs

#cov: h2_test
cov: h2_cov
	head -n -1 h2_report.html > report.html
#	tail -n +2 qurt_report.html >> report.html

h2_cov:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) all
#	$(MAKE) -C ucos sim 2>&1 | tee make.log
#	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) coverage_report
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) cov.rpt
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) h2_report.html

.PHONY: check-fail test-check cov-check cov_fns

check-fail test-check cov-check:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) TESTOUT=$(TESTOUT) check-fail
#	$(MAKE) -C ucos check

check:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) TESTOUT=$(TESTOUT) check
#	$(MAKE) -C ucos check

doc:
	$(MAKE) -C libs/docs/dox
	$(MAKE) -f scripts/docs/Makefile.sphinx prepare
	$(MAKE) -f scripts/docs/Makefile.sphinx doctest html

compat:
	cd install/lib ; ln -s libh2kernel.a libblastkernel.a ; ln -s libh2.a libblast.a

.PHONY: gtags gtagsclean htags

gtags:
	find booter examples kernel libs linux perf qurt scripts stake tst -path libs/syscall/angel/include -o -path kernel/include -prune -o -path "libs/*/include" -prune -o -type f -print | gtags -I -w -v -f -

htags: gtags
	htags -ahnosTxvF --show-position --auto-completion --tree-view=filetree

gtagsclean:
	rm -rf GPATH GRTAGS GSYMS GTAGS ID HTML

cov_fns:
	# $(MAKE) clean ref ARCHV=v4 OPTIMIZE='-Os -fno-inline';
	# ./scripts/gen_cov_fns.pl > ./scripts/v4ref_cov_fns;
	# $(MAKE) clean opt ARCHV=v4 OPTIMIZE='-Os -fno-inline';
	# ./scripts/gen_cov_fns.pl > ./scripts/v4opt_cov_fns;
	# $(MAKE) clean ref ARCHV=v5 OPTIMIZE='-Os -fno-inline';
	# ./scripts/gen_cov_fns.pl > ./scripts/v5ref_cov_fns;
	# $(MAKE) clean opt ARCHV=v5 OPTIMIZE='-Os -fno-inline';
	# ./scripts/gen_cov_fns.pl > ./scripts/v5opt_cov_fns;
	$(MAKE) clean ref ARCHV=60 OPTIMIZE="$(OPTIMIZE_COV)";
	./scripts/gen_cov_fns.pl > ./scripts/v60ref_cov_fns;
	$(MAKE) clean opt ARCHV=60 OPTIMIZE="$(OPTIMIZE_COV)";
	./scripts/gen_cov_fns.pl > ./scripts/v60opt_cov_fns;
	$(MAKE) clean ref ARCHV=65 OPTIMIZE="$(OPTIMIZE_COV)";
	./scripts/gen_cov_fns.pl > ./scripts/v65ref_cov_fns;
	$(MAKE) clean opt ARCHV=65 OPTIMIZE="$(OPTIMIZE_COV)";
	./scripts/gen_cov_fns.pl > ./scripts/v65opt_cov_fns;
	$(MAKE) clean ref ARCHV=68 OPTIMIZE="$(OPTIMIZE_COV)";
	./scripts/gen_cov_fns.pl > ./scripts/v68ref_cov_fns;
	$(MAKE) clean opt ARCHV=68 OPTIMIZE="$(OPTIMIZE_COV)";
	./scripts/gen_cov_fns.pl > ./scripts/v68opt_cov_fns;

.PHONY: q6testinstallenvs
 q6testinstallenvs:
	export Q6TESTINSTALL_MAKEJOBS=1
	export Q6TESTINSTALL_TESTTARGET='testall'

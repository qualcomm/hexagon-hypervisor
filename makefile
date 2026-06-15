include scripts/Makefile.inc.config
include scripts/Makefile.inc.opensource
include scripts/Makefile.inc.version
include scripts/Makefile.inc.tools

# Enable parallel builds by default.  Use the number of available CPUs so we
# don't overwhelm shared machines.  A user-supplied -jN on the command line
# takes precedence because command-line flags are processed after the Makefile.
ifeq ($(findstring -j,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(shell nproc)
endif

JFLAG ?= -j
TEST_JFLAG ?= -j 8

NULLSTRING :=
define nl

$(NULLSTRING)
endef

tab := $(shell printf '\011')

.PHONY: all
all:
	$(MAKE) $(JFLAG) build

.PHONY: build
build:
ifeq ($(USE_PKW),1)
	@echo PKW_VERSIONS $(PKW_VERSIONS)
	pkw --which $(CC)
endif
	$(MAKE) $(JFLAG) -C kernel ARCHV=$(ARCHV) install && \
	$(MAKE) $(JFLAG) -C libs ARCHV=$(ARCHV) install && \
	$(MAKE) $(JFLAG) -C stake ARCHV=$(ARCHV) install
	$(MAKE) $(JFLAG) -C booter ARCHV=$(ARCHV) install
	cp scripts/Makefile.inc.config $(INSTALLPATH)/scripts
	cp scripts/Makefile.inc.opensource $(INSTALLPATH)/scripts
	cp scripts/devsim_v*.cfg $(INSTALLPATH)/scripts
	$(MAKE) $(JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare;
	echo "v$(ARCHV) $@ ${MAKEFLAGS}" > $(INSTALLPATH)/ver
	echo "sha_short $(H2K_GIT_COMMIT)" >> $(INSTALLPATH)/ver
	echo "sha_long $(H2K_GIT_COMMIT_LONG)" >> $(INSTALLPATH)/ver
	sha256sum $(INSTALLPATH)/lib/libh2kernel.a $(INSTALLPATH)/lib/libh2.a \
	    $(INSTALLPATH)/lib/libh2check.a $(INSTALLPATH)/bin/booter \
	    > $(INSTALLPATH)/manifest.tmp; \
	cmp -s $(INSTALLPATH)/manifest.tmp $(INSTALLPATH)/manifest || \
	    cp $(INSTALLPATH)/manifest.tmp $(INSTALLPATH)/manifest; \
	rm -f $(INSTALLPATH)/manifest.tmp


.PHONY: buildall
$(eval buildall:$(foreach a,$(ARCHV_LIST),$(foreach v,$(VARIANTS),$(nl)$(tab)$$(MAKE) ARCHV=$(a) TARGET=$(v))))

all_clean:
	rm -rf artifacts/

distclean: clean docclean gtagsclean

clean: covclean booterclean docclean qurtclean # ucosclean
	$(MAKE) -C kernel ARCHV=$(ARCHV) clean
	$(MAKE) -C stake ARCHV=$(ARCHV) clean
	$(MAKE) -C libs ARCHV=$(ARCHV) clean
	rm -Rf size test.exe stats.txt artifacts/v$(ARCHV)/$(TARGET) kernel/stats.txt

booterclean:
	$(MAKE) -C booter ARCHV=$(ARCHV) clean

docclean:
	$(MAKE) -C libs/docs/dox ARCHV=$(ARCHV) clean
	$(MAKE) -f scripts/docs/Makefile.sphinx clean

testclean covclean: qurtclean # ucosclean
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) clean && \
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) clean_top

# ucosclean:
# 	$(MAKE) -C ucos clean

qurtclean:
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) clean
	$(MAKE) -f scripts/Makefile.qurt clean_top

sim: ref
	$(CC) -mv$(TOOLARCH) -moslib=h2 -moslib=h2kernel -I$(INSTALLPATH)/include -L$(INSTALLPATH)/lib tst/test.c -o test.exe && \
	$(RUN) $(SIMF) -- test.exe;

size:
	hexagon-objdump -h $(INSTALLPATH)/lib/*.a | $(SIZE_TOOL) text > size && \
	cat size;

# t:
# 	/prj/dsp/qdsp6/arch/scripts/test_h2.pl $(TEST_H2_OPTS)

# All per-variant test result JSON paths — one per ARCHV×variant combination.
ARCHV_VARIANT_JSONS := $(foreach a,$(ARCHV_LIST),$(foreach v,$(VARIANTS),artifacts/v$a/$v/install/test_results.json))

# One rule per ARCHV×variant: 'test_variant' builds and tests a single variant,
# generating test_results.json which testall aggregates into the unified report.
define ARCHV_VARIANT_TEST_RULE
artifacts/v$(1)/$(2)/install/test_results.json:
	$$(MAKE) ARCHV=$(1) TARGET=$(2) test_variant
endef
$(foreach a,$(ARCHV_LIST),$(foreach v,$(VARIANTS),$(eval $(call ARCHV_VARIANT_TEST_RULE,$a,$v))))

# Unified test report: one section per ARCHV×variant, driven by all JSON files.
# Building this target drives the full build+test DAG for every combination.
artifacts/test_report.html: $(ARCHV_VARIANT_JSONS)
	@mkdir -p artifacts
	python3 $(H2DIR)/scripts/gen_test_report.py \
	    --inputs $(ARCHV_VARIANT_JSONS) \
	    --output $@ \
	    --summary-out artifacts/test.out

.PHONY: testall test
testall: artifacts/test_report.html

# 'make test' runs all ARCHV×variant combinations (same as testall).
# Use 'make test_variant' to run a single variant (ARCHV=XX TARGET=yy).
test: testall

# NO_TEST_RESET=1 suppresses TESTOUT truncation in 'make test_variant' when the
# caller wants to accumulate results across multiple runs.
NO_TEST_RESET ?= 0

test_variant:
ifneq ($(NO_TEST_RESET),1)
	@if [ -d "$(dir $(TESTOUT))" ]; then > "$(TESTOUT)"; fi
endif
	$(MAKE) h2_test
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) TESTOUT=$(TESTOUT) test_summary
	$(MAKE) check-fail

h2_test: # ucosclean
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.coverage ARCHV=$(ARCHV) tst 2>&1 | tee $(INSTALLPATH)/make.log; exit $${PIPESTATUS[0]}
#$(MAKE) -C ucos sim 2>&1 | tee make.log
	[ `fgrep -v "WARNING: Overriding currently set revid" $(INSTALLPATH)/make.log | fgrep -v "warning: -j" | fgrep -c -i warning:` -eq 0 ]
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) $(INSTALLPATH)/test_report.html
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) $(INSTALLPATH)/test_results.json

qurt_test: ./qurt/test/testcases
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.qurt ARCHV=$(ARCHV) tst 2>&1 | tee $(TESTOUT); exit $${PIPESTATUS[0]}
#	[ `fgrep -c -i warning: $(TESTOUT)` -eq 0 ]
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) $(INSTALLPATH)/qurt_report.html

qurt_test_single: ./qurt/test/testcases
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) prepare
	$(MAKE) $(TEST_JFLAG) -f scripts/Makefile.qurt ARCHV=$(ARCHV) TEST=$(TEST) tst_single 2>&1 | tee $(TESTOUT); exit $${PIPESTATUS[0]}
	[ `fgrep -c -i warning: $(TESTOUT)` -eq 0 ]

qurt_test_libs:
	$(MAKE) -f scripts/Makefile.qurt ARCHV=$(ARCHV) qurt_test_libs

#cov: h2_test
cov: h2_cov
#	tail -n +2 qurt_report.html >> report.html

h2_cov:
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) $(INSTALLPATH)/cov.rpt
	$(MAKE) -f scripts/Makefile.coverage ARCHV=$(ARCHV) $(INSTALLPATH)/test_report.html

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

LLDB_PORT ?= 1234

.PHONY: lldb-setup lldb-setup-clean
lldb-setup:
	$(MAKE) -f scripts/Makefile.lldb_setup LLDB_PORT=$(LLDB_PORT)

lldb-setup-clean:
	$(MAKE) -f scripts/Makefile.lldb_setup clean

.PHONY: vscode vscode_clean
vscode:
	$(MAKE) -f scripts/Makefile.vscode TOOLS_FLAVOR=$(TOOLS_FLAVOR) ARCH_FLAVOR=$(ARCH_FLAVOR) ARCHV=$(ARCHV) TARGET=$(TARGET)

vscode_clean:
	$(MAKE) -f scripts/Makefile.vscode clean

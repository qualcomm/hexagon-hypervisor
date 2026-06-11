# H2 Hexagon Hypervisor - Project Notes

## Project Summary

H2 is a minimal, efficient hypervisor/monitor-mode software for Qualcomm Hexagon
processors.  It runs in Monitor Mode (the most privileged mode), supports
multiple hardware threads, and provides both RTOS and VM-style services.

## Architecture Overview

### Privilege Levels (Hexagon)
- **Monitor Mode**: Most privileged.  H2 kernel runs here.
- **Guest Mode**: Guest OS / RTOS threads run here.
- **User Mode**: User-space threads.

Entry into monitor mode happens via: interrupt, trap0 (RTOS), trap1 (VM/hypervisor),
exception, or reset.  All cause the processor to jump to the Event Vector Base (EVB).

### Hardware Thread Support
The processor has multiple hardware threads (hthreads).  Each hthread has its own
kernel stack, per-hardware-thread data in `H2K_kg_t`, and a runlist entry.

### Global Pointer
The kernel global state (`H2K_kg_t`) is accessed via r28, held in `H2K_gp`.
This is a hardware register convention -- r28 is dedicated for the kernel global
pointer.

## Key Data Structures

### `H2K_kg_t` (kernel globals, kernel/include/globals.h)
- Holds all global kernel state: ready list, runlist, interrupt handlers, ASID table,
  futex hash table, VM block array, TLB state, timer info, coprocessor state, etc.
- One instance: `H2K_kg`, in `.data.core.globals` section.
- Accessed via r28 (`H2K_gp`).

### `H2K_thread_context` (kernel/include/context.h)
- Per-thread context: ~288+ bytes, 32-byte aligned.
- Contains: kernel variables (next/prev ring pointers, tid, hthread, prio, status),
  VM info (vmblock, id), saved registers (r0-r31, predicates, loop regs, guest regs),
  continuation pointer, ccrssr (CCR + SSR), futex state, tree node for timeout.
- Status values: DEAD, RUNNING, READY, BLOCKED, VMWAIT, INTBLOCKED.

### `H2K_ext_context`
- Extended (HVX vector) register context.  Stored separately from main context.
- Contains vregs (vector registers) and qregs (predicate vector registers).

### `H2K_fastint_context`
- Fast interrupt context: a thread context + small kernel stack (16 u64 words).

## Key Mechanisms

### Continuation-Based Context Switch
Instead of per-user-thread kernel stacks, there is a per-hardware-thread kernel stack.
Return from kernel is defined by a "continuation" pointer stored in the thread context.
When switching back to a thread, H2K_check_sanity is called, then the continuation.

### Scheduler (`kernel/sched/`)
- `dosched`: picks the best-priority ready thread, handles low-priority thread
  selection, and calls `H2K_switch`.
- `readylist`: priority queue of runnable threads (up to 256 priority levels).
- `runlist`: per-hthread currently running thread.
- `switch`: performs context save/restore.
- `lowprio`: marks/unmarks the lowest priority thread to manage interrupt masking.
- `yield`, `resched`, `check_sanity`: other scheduling support.

### Interrupt Handling (`kernel/event/`)
Three interrupt categories:
1. **Reschedule interrupt**: causes lowest-priority thread to reschedule.
2. **Fast interrupts**: run in monitor mode, very limited functionality
   (non-blocking only; futex resume recommended).  Have their own context.
3. **VM-style interrupts**: delivered as VM events to guest threads.

### Memory Management (`kernel/mem/`)
- TLB management: fill, miss handling, ASID management.
- STLB: software TLB cache (sets/ways structure) for faster TLB miss handling.
- Page walk: walks guest page tables, validates mappings, converts TLB misses to
  page faults when no valid translation found.
- Physical memory reads (`physread`): for safely reading physical addresses.
- Safe memory access (`safemem`): verifies guest/user addresses before access.
- `alloc`: kernel heap allocator.
- `linear`: linear allocator (simpler).
- `tmpmap`: temporary mapping for kernel access to arbitrary physical pages.

### Virtual Machine Support (`kernel/data/vm/`, `kernel/include/vm*.h`)
- `H2K_vmblock_t`: per-VM state (trap mask, memory ranges, etc.)
- Trap handling: trap0 -> RTOS services; trap1 -> hypervisor services.
- trap0 from user mode: forwarded as event to guest.
- trap1 from user mode: not allowed (converted to guest event).

### Futex (`kernel/futex/`)
- Classic and PI (priority inheritance) variants.
- Uses hash table in `H2K_kg_t` for blocked threads.
- Address checked via TLB lock before access.

### Boot Process (`kernel/init/boot/`)
1. Clear caches, set syscfg.
2. Set up physical-to-virtual TLB entries for kernel pages.
3. Map L2 interrupt controller and angel page.
4. Enable MMU (SYSCFG_M bit), jump to virtual address.
5. Call `H2K_thread_boot` (C code) to continue initialization.
Multicore: cores > 0 copy the boot image to their memory region, then synchronize.

## Build System

- Correct workflow: `make opt` then `make test` (or `make opt test`). No `-j` needed —
  `makefile` sets `MAKEFLAGS += -j$(nproc)` automatically; override with `make -j8` if needed.
- `make test` alone will auto-build if `install/` is missing, but defaults to `ref` model.
- `make opt` / `make ref`: optimized or reference builds.
- `ARCHV=xx`: select architecture version (65, 68, 73, 81 are primary).  Default is 81.
- `make.inc` files scattered through the tree are auto-discovered and included.
- Per-architecture optimized assembly files (e.g., `dosched.v81opt.S`).
- Final artifacts in `artifacts/v$(ARCHV)/$(TARGET)/install/`: `bin/booter`, `lib/libh2.a`, etc.
  Each ARCHV+variant (opt/ref) has its own install directory.
- Intermediate .o files in `kernel/build/obj/v$(ARCHV)/opt|ref/` and similarly for libs.
  Ref archives now in `kernel/build/ref_v$(ARCHV)/` and `libs/*/build/ref_v$(ARCHV)/`.
  This allows parallel builds across ARCHV and opt/ref without file conflicts.
- `booter`: binary that loads other programs into VMs for execution (used for tests).
- Tests run via archsim (not hexagon-sim) for ARCHV >= 5 when USE_PKW=1.
- 92 tests in ARCHV=81 testlist; all passed as of 2026-04-23.
### Build Isolation Status (as of 2026-04-24, tasks 1-6 complete)

The out-of-tree build is complete for opt builds. Items 1-4 below are fixed. Item 5
(unified artifact directory) is the next planned task and will consolidate all remaining
in-source build artifacts.

1. **Ref library archives** — FIXED (2026-04-23): Renamed `build/ref/` → `build/ref_v$(ARCHV)/`
   in `kernel/Makefile`, `libs/h2/Makefile`, `libs/h2_compat/Makefile`, `libs/posix/Makefile`,
   `libs/Makefile`, and `scripts/Makefile.inc.test`.  Now matches the opt pattern.

2. **`asm_offsets.h`** — FIXED (2026-04-23): Now generated into `$(BUILD_DIR)/include/asm_offsets.h`
   (i.e., `build/obj/v$(ARCHV)/opt|ref/include/`). `kernel/Makefile` adds `-I$(BUILD_DIR)/include`
   to CFLAGS after BUILD_DIR is defined. STANDALONE tests (`scripts/Makefile.inc.test`) also
   get `-I ${INSTALLPATH}/include` added. The install steps copy it to `$(INSTALLPATH)/include/`
   as before.  `make clean` removes it via `build/obj` and `include/*` (for any stale copy).
   All 92 tests pass with ARCHV=81.

3. **`libs/Makefile` tmp directory** — FIXED (2026-04-23): Introduced `TMP_DIR := tmp/$(IMPL)_v$(ARCHV)`
   (e.g., `libs/tmp/opt_v81/`). Combined `libh2.a` is now built in `$(TMP_DIR)/libh2.a` and
   installed via `cp`; the install target uses `$(TMP_DIR)/libh2.a` as its prerequisite.
   Also fixed `libs/h2/Makefile`: added `-I$(INSTALLPATH)/include` first in CFLAGS so
   `asm_offsets.h` (now only in BUILD_DIR and INSTALLPATH) is found correctly, and added
   `common/asm_offsets.h` to the clean step to remove stale copies.
   All 92 tests pass with ARCHV=81.

4. **Test build artifacts are in-source.** Tests build object files, ELFs, and log
   files directly in their source directories. Parallel multi-arch test runs will
   conflict. Low priority — address after library build isolation is complete.
   FIXED (2026-04-23): Test artifacts now land in `artifacts/v$(ARCHV)/$(TARGET)/build/tests/<test-path>/`.
   `Makefile.coverage` creates a build dir per test, symlinks all source files (excluding
   `Makefile.inc`), generates a `Makefile.inc` with absolute H2DIR, and runs `make -C $$BDIR`.
   `clean` target now does `rm -rf $(TEST_BUILD_ROOT)` instead of per-subdir iteration.
   `check-fail`, `check`, `test_summary`, `report.html` updated to reference build dir paths.
   Also fixed:
   - `libs/Makefile`: removed premature `cp` to TMP_DIR that caused the combination step to
     be skipped; now uses absolute paths in `hexagon-ar x` loop so `$(TMP_DIR)/libh2.a`
     is only ever created by the final `hexagon-ar cru` step.
   - `Makefile.inc.test` `test.log` rule: added `> $@` truncation to prevent old FAIL content
     from contaminating the log on re-runs.
   - 3 test Makefiles updated to use absolute `$(H2DIR)/kernel/include` instead of relative
     `../../../include` (error/test, boot/test; check_sanity's was a no-op either way).
   - `vectors/test/Makefile`: changed `../vectors.ref.o` to `vectors.ref.o` with explicit
     compile rule sourcing from `$(H2DIR)/kernel/event/vectors/vectors.ref.S`.
   - `info/test/Makefile`: moved `CFLAGS += -DH2K_KERNEL_PGSIZE=...` from Makefile.inc.
   - `find_match/Makefile`: added `BOOTER_ARGS := --num_vcpus 512` (was in Makefile.inc chain).
   - `h2_compat/error/test/simple/Makefile`: moved `BOOTER_ARGS` and `POST_RUN` from Makefile.inc.
   All 92 tests pass with ARCHV=81.

5. **Remaining in-source builds** — NOT YET FIXED.  Three components still write build
   artifacts into source directories, which will conflict in parallel ARCHV builds:
   - `booter/` — all .o files and final binaries land in `booter/` source dir.
   - `libs/qurt/src/` — .o files and `libs/qurt/libqurt.a` land in-source.
   - `libs/syscall/angel/src/` — .o files and `libs/syscall/angel/libangel.a` land in-source.
   Additionally, `make clean` does not remove stale in-source .o files left over from
   pre-migration builds (kernel/**/*.ref.o, libs/**/*.ref.o, etc.) or old `build/ref/`
   archive directories.  These will all be resolved by the unified artifact dir (item 6).

6. **Unify all build artifacts under `artifacts/`** — DONE (2026-04-24).
   See "Unified Build Directory Plan" section below for the completed work.

## Unified Build Directory Plan (COMPLETED 2026-04-24)

**Goal achieved:** All intermediate build artifacts (object files, intermediate archives,
booter/qurt/angel .o files) are now under `artifacts/v$(ARCHV)/$(TARGET)/build/`.

**Approach:** `BUILD_DIR := $(patsubst %/install,%/build/COMPONENT,$(INSTALLPATH))` in each
Makefile. For kernel, an `ifeq ($(findstring /ref/,$(BUILD_DIR)),/ref/)` conditional picks
REF_OFILES vs OPT_OFILES for the archive. `libh2check.a` is built as part of both
`ref_install` and `opt_install` and installed directly to `$(INSTALLPATH)/lib/`.

**Files changed:**
- `kernel/Makefile` — BUILD_DIR from INSTALLPATH; single archive target with ifeq; check built alongside kernel; clean simplified.
- `libs/h2/Makefile`, `libs/posix/Makefile`, `libs/h2_compat/Makefile` — same pattern.
- `libs/Makefile` — TMP_DIR, H2_LIB, POSIX_LIB, H2_COMPAT_LIB, QURT_LIB, ANGEL_LIB all derived from INSTALLPATH; HEADER_DIRS for source-dir includes.
- `booter/Makefile` — BUILD_DIR; BOOTVM_PROG is now an absolute path (`$(BUILD_DIR)/booter`).
- `scripts/Makefile.inc.bootvm` — `_BOOTVM_BUILD_DIR` with fallback to `.`; all intermediates under BUILD_DIR; BOOTVM_PROG_TMP simplified.
- `stake/makefile` — BUILD_DIR; kernel_tmp and vapa_kernel_tmp.x moved to BUILD_DIR.
- `libs/syscall/angel/Makefile` — BUILD_DIR; O_FILES prefixed; explicit compile rules.
- `libs/qurt/Makefile` — same.
- `scripts/Makefile.inc.test` — LIBH2CHECK fixed to `${INSTALLPATH}/lib/libh2check.a`; old broken rule removed.

**Stale in-source artifacts** (pre-migration, not harmful, will disappear with `git clean -fdx`):
- `booter/*.o`, `booter/booter` (pre-migration binaries)
- `kernel/**/*.{ref.o,v81opt.o,...}`, old `build/ref/` dirs
- `libs/**/*.ref.o`, `libs/sos/`

### Pre-parallel-build cleanup (2026-04-24)

Removed all `git clean -dxf` calls from `testall` — they were added before artifacts were
isolated and would wipe untracked files (notes.md, CLAUDE.md, docs/, etc.).
`testall_prepare` now only resets `test.out`; the per-ARCHV clean calls are gone entirely.

Moved root-level output files into the per-variant artifact directory, using proper
file-based make targets so parallel ARCHV builds write to distinct paths with no conflicts:
- `make.log` → `$(INSTALLPATH)/make.log` (h2_test pipes tst output there directly)
- Added `$(INSTALLPATH)/test_report.html` target to `Makefile.inc.test` that writes
  directly to `$@` using `$@` throughout — no intermediate root-level file.
  The old `report.html` / `h2_report.html` root targets are preserved for the qurt path.
- `makefile` h2_test now calls `make $(INSTALLPATH)/test_report.html` directly;
  `report.html` also written to `$(INSTALLPATH)/report.html`.
`clean_top` in `scripts/Makefile.inc.test` updated to drop the now-gone root-level files.

### Individual ARCHV opt validation (2026-04-24) — COMPLETE

All four opt variants pass:
- v65/opt: 83 passed, 0 failed
- v68/opt: 83 passed, 0 failed
- v73/opt: 83 passed, 0 failed  (was 1 failure — pingpong stack overflow; fixed in same session)
- v81/opt: 92 passed, 0 failed

v73 has more tests than v65/v68 (83 vs 83 — same), v81 has 9 additional tests.

### Individual ARCHV ref validation (2026-04-24) — COMPLETE

All four ref variants pass (after `rm -rf artifacts/ install/ kernel/build/obj/` clean):
- v65/ref: 83 passed, 0 failed
- v68/ref: 83 passed, 0 failed
- v73/ref: 83 passed, 0 failed
- v81/ref: 92 passed, 0 failed

The earlier ref failure (noted in previous session) was a stale artifact issue —
after a clean build, ref works correctly for all variants.

**DONE (2026-04-27):** File-based DAG for test targets — see section below.

### Validated single-variant workflow (2026-04-24)

Running opt and ref individually works correctly for all four variants.
The correct invocation pattern is:

    make ARCHV=XX TARGET=opt opt   # or: make ARCHV=XX opt
    make ARCHV=XX TARGET=opt test

    make ARCHV=XX TARGET=ref ref   # or: make ARCHV=XX ref
    make ARCHV=XX TARGET=ref test

Added `make all_clean` target (nukes the entire `artifacts/` directory) to
complement the per-variant `make clean` (which only removes `artifacts/v$(ARCHV)/$(TARGET)`).

### testall fix (2026-04-27) — FIXED AND VALIDATED

Two bugs fixed:

1. **`makefile` `$(ARCHV_LIST)` recipe**: split `all test` into two separate `$(MAKE)` lines
   (`opt` then `test`, `ref` then `test`) so there is no ordering ambiguity under forced `-j`.

2. **`Makefile.inc.test` `${INSTALLPATH}` fallback**: changed from backslash-continuation
   with `+$(MAKE) ${MODEL}` to a single `cd ... && $(MAKE) $(TARGET)` line.  Fixes two
   sub-bugs: `+` on a continuation line is treated as a literal shell character (not a make
   modifier), and `MODEL` was always `ref` regardless of `TARGET`.

### testall failure analysis (2026-04-24) — NOT YET FIXED

`make testall` fails with `Error 127` ("command not found") on all four ARCHV
targets.  The failure chain:

    make[3]: *** [scripts/Makefile.inc.test:110: .../v65/opt/install] Error 127

The recipe at `Makefile.inc.test:109-111` is the `${INSTALLPATH}` auto-build
fallback — triggered when a test's dependency on `${INSTALLPATH}` fires and the
directory is not yet present:

    ${INSTALLPATH}:
        cd ${H2DIR};\
        +$(MAKE) ${MODEL} ARCHV=${ARCHV}

Two bugs in this recipe:

1. **`+` prefix on continuation line causes `+make: command not found`.**
   The `+` recipe modifier (jobserver participation) is only recognized by make
   at the *start of a logical recipe line*.  Because the line is a backslash
   continuation of `cd ${H2DIR};\`, the shell receives the literal string
   `+make ref ARCHV=65`, and `/bin/bash` reports "command not found".

2. **`MODEL` defaults to `ref` regardless of `TARGET`.**
   In `Makefile.inc.test`, `MODEL` is set to `ref` when unset (lines 34-35).
   During a `TARGET=opt` build, `MODEL` is never set to `opt`, so the fallback
   would try to build `ref` even when the opt install dir is needed.

**Root cause of why the fallback fires at all:**
The `testall` rule invokes `$(MAKE) ARCHV=$@ TARGET=opt ... all test` as a single
sub-make.  The Makefile unconditionally sets `MAKEFLAGS += -j$(nproc)`.  When
this sub-make runs as a child of the `-j$(nproc)` top-level make, it triggers
`make[2]: warning: -j40 forced in makefile: resetting jobserver mode.`  Under
this reset, the `prepare` step of `test` appears to start *before* `all`
finishes (prepare is seen at line 44 of the log while opt_install starts at
line 59).  This means `${INSTALLPATH}` doesn't exist yet when the test
Makefile tries to use it, so the fallback rule fires.

The underlying question (to confirm in a fresh session): does GNU Make with
`MAKEFLAGS += -jN` forced in the Makefile actually run command-line goals
`all test` in parallel rather than sequentially?  If yes, that is the true
root cause; the `+` and `MODEL` bugs are secondary.

**Fix approach (to implement next session):**
- The `testall` rule should pass `$(MAKE) ARCHV=$@ TARGET=opt ... opt` (the
  explicit build target) and `$(MAKE) ARCHV=$@ TARGET=opt ... test` as two
  *separate* recipe steps, so there is no ambiguity about ordering.
- OR: ensure `all test` command-line goals are truly sequential even under
  forced `-j` — potentially by removing `MAKEFLAGS += -j$(nproc)` and using
  `JFLAG` more carefully so it doesn't propagate to contexts where it causes
  goal-ordering problems.
- Also fix the `${INSTALLPATH}` fallback recipe: remove the `+` prefix from
  the continuation line, and make it use `$(TARGET)` or `TARGET` rather than
  `MODEL` so it builds the correct variant.

### Potential future improvement: file-based make DAG

DONE (2026-04-27) — see section below.

Remaining in-source issue (FIXME in Makefile.qurt): per-test qurt support libraries
(`$(QURT_TEST_LIBS)::`) still build in-source via `cd $@`. Needs the same out-of-tree
treatment as the per-test $(SUBDIRS) rule. Low priority — qurt test source doesn't
exist in this repo yet.

### File-based DAG for test targets (2026-04-27) — DONE

**Goal:** Replace phony `tst` and always-rebuilding test rules with a real file-based
DAG so Make can skip up-to-date tests and `-j` scheduling is explicit.

**Changes to `scripts/Makefile.coverage`:**
- `RESULT_FILES` variable: list of `$(TEST_BUILD_ROOT)/%/results.txt` paths (one per test).
  Defined with `=` (lazy expansion); `TEST_BUILD_ROOT` defined early in the file.
- `tst: $(RESULT_FILES)` — phony target with no recipe; depends on real file targets.
- Pattern rule `$(TEST_BUILD_ROOT)/%/results.txt: $(INSTALLPATH)/ver`:
  does the setup (mkdir, symlinks, Makefile.inc) + sub-make + `touch $@` on success.
- Old `$(SUBDIRS)::` double-colon rules kept for `TARG=all` (coverage) path only.

**Changes to `scripts/Makefile.inc.test`:**
- `$(INSTALLPATH)/test_report.html` and `$(INSTALLPATH)/qurt_report.html` now
  depend on `$(RESULT_FILES)` — only regenerated when a result file changes.
- `$(INSTALLPATH)/ver:` rule added — triggers the full build if `ver` is missing
  (mirrors the existing `$(INSTALLPATH):` auto-build fallback).

**Key semantic:** `$(INSTALLPATH)/ver` is written by every `make opt`/`make ref` run.
If `ver` is newer than a test's `results.txt`, the test is re-checked (sub-make is
invoked). If the inner make finds nothing to rebuild, `touch $@` stamps the result
to record "validated against this build." If `ver` hasn't changed since the last
test run, the entire `tst` is a no-op.

**Verified behavior (2026-04-27):**
- `make opt test`: 92 passed, 0 failed (v81/opt)
- Repeat `make test` (no new `make opt`): `make[2]: Nothing to be done for 'tst'.` — true no-op
- After `touch ver` (simulate rebuild): tests re-check but simulators don't re-run;
  `touch $@` stamps results; subsequent `make test` is again a no-op

**DONE (2026-04-28):** Replaced `$(INSTALLPATH)/ver` with `$(INSTALLPATH)/manifest` as the
test DAG sentinel. The manifest holds sha256sums of `libh2kernel.a`, `libh2.a`,
`libh2check.a`, and `booter`. Generated at the end of both `opt` and `ref` targets using
update-if-changed (write to `.tmp`, `cmp -s`, only `cp` if different). This means a
no-op rebuild that produces identical binaries will not update the manifest and tests will
stay a true no-op. `$(INSTALLPATH)/manifest:` fallback rule added to `Makefile.inc.test`.

**DONE (2026-04-28):** Fixed silent failure escapes and bumped ULIMIT:
- `check-fail` in `Makefile.inc.test`: for loop's `exit 1` was swallowed by `| tee -a $(TESTOUT)`.
  Fixed by wrapping in `{ }` and appending `; exit $${PIPESTATUS[0]}`.
- `h2_test` in `makefile`: `$(MAKE) tst 2>&1 | tee make.log` swallowed make failure exit code.
  Fixed with `; exit $${PIPESTATUS[0]}`.
- `qurt_test` and `qurt_test_single` same fix.
- `ULIMIT` bumped from 300 → 600 seconds (needed for parallel testall load).
  `kernel/traps/pmu/test_h2` was hitting the 300s CPU limit under full 8-way parallel load.

**DONE (2026-04-28):** Updated `docs/build_and_test.md`: marked completed todo items as `[x]`, updated
Output Files section to reference `artifacts/` paths instead of root-level files, replaced
`install/ver` with `install/manifest` description.

**DONE (2026-04-28):** Unified test report — `artifacts/test_report.html` is a file-based
target depending on per-variant `test_results.json` files (one per ARCHV×variant combination).
`testall` is now a phony alias for `make artifacts/test_report.html`. Per-variant JSON is
generated by `scripts/gen_test_results.py`; unified HTML by `scripts/gen_test_report.py`.
`make test` auto-builds via the manifest fallback, so the ARCHV_VARIANT_RULE only needs
`$(MAKE) ARCHV=$(1) TARGET=$(2) test` — no explicit build step in the rule.

Fix required: `> $(TESTOUT)` in `test` target was unconditional, failing when INSTALLPATH
didn't exist yet. Changed to `if [ -d "$(dir $(TESTOUT))" ]; then > "$(TESTOUT)"; fi` so
the truncation is skipped on a clean build (prepare will build INSTALLPATH first, then
test_summary creates TESTOUT fresh). The old `mkdir -p` approach broke `prepare`'s
`${INSTALLPATH}:` dependency check, causing build output to land in `make.log`.

Validated 2026-04-28 from clean (`make all_clean && make artifacts/test_report.html`):
- v65/opt: 83 passed, 0 failed
- v65/ref: 83 passed, 0 failed
- v68/opt: 83 passed, 0 failed
- v68/ref: 83 passed, 0 failed
- v73/opt: 83 passed, 0 failed
- v73/ref: 83 passed, 0 failed
- v81/opt: 92 passed, 0 failed
- v81/ref: 92 passed, 0 failed
Total: 682 passed, 0 failed → artifacts/test_report.html (50 KB)

TODO: add top-level summary table + parseable combined JSON artifact to gen_test_report.py.

**NEXT TASK:** Consider committing the branch. Remaining open items:
- Phase 2 DAG (per-component selective re-runs)
- Top-level summary table + combined JSON in gen_test_report.py
- Dead code cleanup in `Makefile.inc.test` (old `report.html`/`h2_report.html`/`qurt_report.html` targets ~lines 249-280)

### Parallel build race condition fixes (2026-05-14, completed 2026-05-15) — DONE

Two race conditions fixed, stale artifacts and gitignore entries cleaned up.
Verified: 682 passed, 0 failed across all 8 variants after `make all_clean && make testall`.

#### Remaining stale source-tree include paths fixed (2026-05-15)

The b7806811a commit moved kernel/lib headers into per-variant `BUILD_DIR/include/`
but missed three Makefiles that still referenced old source-tree paths:

- **`libs/syscall/angel/Makefile`**: `CFLAGS`/`ASFLAGS` used `../../h2/include`,
  `../../h2_compat/include`, `../../posix/include`.  Added `H2_BUILD_DIR`,
  `H2_COMPAT_BUILD_DIR`, `POSIX_BUILD_DIR` derived from `INSTALLPATH` (same pattern
  as `libs/posix/Makefile`); updated both flag sets to use them.

- **`libs/qurt/Makefile`**: Same three stale paths embedded directly in the `CC`
  definition.  Added same three `BUILD_DIR` vars; updated `CC`.

- **`booter/Makefile`** + **`booter/booter.c`**: Makefile used `-I$(KERNELPATH)/include`
  (source-tree `kernel/include/`, no longer exists).  Added
  `KERNEL_BUILD_DIR := $(patsubst %/install,%/build/kernel,$(INSTALLPATH))` and
  replaced the flag.  `booter.c` had `#include "../kernel/include/max.h"` and
  `#include "../kernel/include/hw.h"` as relative paths bypassing `-I`; changed both
  to `<max.h>` and `<hw.h>` (found via the new `-I$(KERNEL_BUILD_DIR)/include`).
  `max.h`, `hw.h`, and `asm_std.h` (needed by `bootvm_entry.S`) all have `make.inc`
  files in their kernel module dirs and are copied to `BUILD_DIR/include/` by the kernel build.

#### Remaining stale source-tree include paths fixed (2026-05-15, part 2)

Three more places used `${H2DIR}/kernel/include` (the old source-tree generated dir):

- **`scripts/Makefile.inc.test:158`** `KERNEL_INCLUDES`: changed from
  `-I ${H2DIR}/kernel/include` to `-I $(KERNEL_BUILD_DIR)/include` where
  `KERNEL_BUILD_DIR := $(patsubst %/install,%/build/kernel,$(INSTALLPATH))`.
  This affects STANDALONE test builds.  For non-STANDALONE kernel tests that add
  their own `-I$(H2DIR)/kernel/include`, two test Makefiles also needed fixing:

- **`kernel/event/error/test/Makefile`** and **`kernel/init/boot/test/Makefile`**:
  both had `CFLAGS += -ffixed-r28 -I$(H2DIR)/kernel/include` (and ASFLAGS for
  error/test).  Changed to derive `KERNEL_BUILD_DIR` from `INSTALLPATH` locally
  and use `-I$(KERNEL_BUILD_DIR)/include`.  These tests include `<asm_std.h>`,
  `<c_std.h>` etc. from kernel/util/ which are only in the per-variant build dir.

`make test` → `make testall` alias:
- Renamed per-variant `test:` recipe to `test_variant:` (still useful for single-variant testing).
- Updated `ARCHV_VARIANT_RULE` to call `test_variant` instead of `test`.
- Added `test: testall` so `make test` now runs all 8 variants.

Validated: `make all_clean && make test` → 682 passed, 0 failed across 8 variants.

#### include/ race condition — FIXED

`kernel/include/`, `libs/h2/include/`, `libs/posix/include/`, `libs/h2_compat/include/`
were gitignored generated directories in the source tree.  All 8 parallel builds wrote
to the same shared paths.

**Fix:** Changed all four `build/make/make.inc.default` files to set `DEST_HFILES` and
`DEST_HFILE` under `$(BUILD_DIR)/include/` instead of the relative `include/`.  Updated
each sub-lib `Makefile` to use `-I$(BUILD_DIR)/include` in `CFLAGS` (added after `BUILD_DIR`
is defined).  Cross-library references in `libs/posix` and `libs/h2_compat` use
`H2_BUILD_DIR` / `POSIX_BUILD_DIR` derived from `INSTALLPATH`.  Updated `libs/Makefile`
`install` target to tar headers from the per-variant `BUILD_DIR`-based paths instead of
`dir/include/` for h2/posix/h2_compat (syscall/angel and qurt are source-controlled, unchanged).
Removed the four source-tree include dirs from `.gitignore`.  Added `include` to each
sub-lib `clean` target to sweep stale dirs.

#### bootvm incbin race condition — FIXED

`stake/bootvm.S` had `.incbin "bootvm_image.bin"` (relative path).  The actual binary is
at `$(BOOTVM_IMAGE_BIN)` = `BUILD_DIR/bootvm_image.bin`.  Parallel builds could use the
wrong or a stale binary.

**Fix:** Changed `stake/bootvm.S` to `.incbin BOOTVM_IMAGE_BIN_PATH`.  Added
`-DBOOTVM_IMAGE_BIN_PATH='"$(BOOTVM_IMAGE_BIN)"'` to the `$(BOOTVM_O)` rule in
`scripts/Makefile.inc.bootvm` so the C preprocessor (which runs first on `.S` files)
expands the macro to the absolute per-variant path before the assembler sees `.incbin`.

#### asm_offsets.h — already safe

`$(BUILD_DIR)/include/asm_offsets.h` is generated by running `$(BUILD_DIR)/offsets`
(built from `kernel/build/offsets/offsets.ref.c`).  All paths are under `BUILD_DIR`
(per-ARCHV/variant) — no conflict.  The `libs/h2/Makefile` clean had a dead
`common/asm_offsets.h` entry (never generated); removed it.

#### Stale source-tree artifacts and gitignore cleaned up

Deleted stale binaries/objects from `booter/`, `kernel/offsets`, `stake/kernel_tmp`,
`stake/vapa_kernel_tmp.x` (all pre-migration, now in `BUILD_DIR`).  Removed 14 now-stale
gitignore entries: the four `*/include` dirs, `kernel/offsets`, all `booter/` build
artifacts, and the two `stake/` intermediates.

## Code Coverage (Legacy gmon approach)

### Overview

Two coverage mechanisms exist in the codebase:

1. **Legacy gmon-based** (working): The Hexagon simulator produces `gmon.out` /
   `gmon-0x*.out` profiling files during test runs.  `hexagon-coverage` converts these
   to `cov.txt` per-test, `merge_cov.py` aggregates them, and `cov_rpt_tool` produces
   the human-readable `cov.rpt`.  This works for both C and assembly.

2. **LLVM instrumentation** (partially wired, not working): `TARGET=llvm_cov` adds
   `-fprofile-instr-generate -fcoverage-mapping` to CFLAGS, producing `default.profraw`
   files.  The merge/report targets exist in `Makefile.coverage` but are incomplete:
   they only instrument STANDALONE test binaries (not the kernel/libs), and the HTML
   report target is commented out as a TODO.  Assembly code is not instrumented by LLVM,
   which is a fundamental limitation given how much asm H2 has.

### How to run legacy coverage

```
make TARGET=opt_cov opt cov ARCHV=81
```

This:
1. Rebuilds with `-fno-inline` (`OPTIMIZE_COV`)
2. Runs all tests via `h2_cov` → `Makefile.coverage all` (TARG=all per test), generating
   gmon files and per-test `cov.txt` in `artifacts/v81/opt/build/tests/<test-path>/`
3. Merges all per-test `cov.txt` files into a root-level `cov.txt` via `merge_cov.py`
4. Generates `cov.rpt` via `cov_rpt_tool`
5. Generates `$(INSTALLPATH)/test_report.html`

### Bugs fixed to get coverage working (2026-04-28)

All three were fallout from the out-of-tree build migration:

1. **`Makefile.inc.test:237` `find ./kernel ./libs`**: Per-test `cov.txt` files now
   land in `TEST_BUILD_ROOT` (not in-source).  Changed to `find $(TEST_BUILD_ROOT)`.
   Also added `--dir $(H2DIR) --installpath $(INSTALLPATH)` args to the `merge_cov.py` call.

2. **`merge_cov.py` hardcoded `h2dir+"/install/ver"`**: Added `--installpath` option
   to the script; defaults to `h2dir+"/install"` for backward compat.  The Makefile
   now passes `--installpath $(INSTALLPATH)`.  (The `--dir` arg is for the `scripts/`
   directory and is separate.)

3. **`merge_cov.py` `nop_patt.match().group(1)` crash**: `nop_patt` does not match
   every instruction; `match` can be `None`.  Added `match and` guard before
   `match.group(1)`.  Pre-existing bug; not triggered until coverage was actually run.

### v81/opt coverage results (2026-04-28, 220 tracked kernel functions)

- **133 functions at 100%** (60%)
- **77 functions at 0%** (35%) — see list in `cov.rpt`
- **10 functions partially covered** (1–99%)

Notable 0% functions (potential test gaps):
`H2K_asid_match`, `H2K_asid_table_eviction`, `H2K_fatal_crash`, `H2K_futex_cancel`,
`H2K_handle_nmi`, `H2K_handle_rsvd`, `H2K_vmop_boot`, `H2K_vmop_free`, `H2K_vmop_status`,
`H2K_mem_physread_dword`, `H2K_intpool_cancel`, `H2K_tree_remove`, `H2K_varadix_update`,
and many low-level accessor stubs (`H2K_get_*`, `H2K_set_*`, `H2K_dccleana`, etc.)

Some 0% functions are expected to be hard to reach (NMI handler, fatal paths, rsvd traps).
Others like `futex_cancel`, `vmop_boot/free/status`, `tree_remove` may indicate real gaps.

### Known issues / noise

- **merge_cov.py diagnostic messages go to stdout**, not stderr.  They get mixed into
  `cov.txt` and then into `cov.rpt` (where `cov_rpt_tool` ignores non-matching lines).
  Should redirect those `print()` calls to `sys.stderr`.

- **`cov.txt` not removed on Make failure** (no `.DELETE_ON_ERROR`).  If `merge_cov.py`
  fails, a zero-byte `cov.txt` is left behind.  Make won't rebuild it next run.
  Workaround: `rm cov.txt` before retrying.  Fix: add `.DELETE_ON_ERROR` or check for
  empty output.

- **"additional pc / parse bit mismatch" warnings**: The `--offset_pc` normalization
  subtracts the function's load address to produce position-independent PCs.  When
  different tests link the kernel at different addresses and the code layout differs
  slightly (tail calls, code folding), the normalized PCs don't always match across
  files.  Pre-existing issue; data is still useful.

- **Coverage pipeline not integrated with `testall`**: `make testall` runs opt+ref for
  all four ARCHVs but does not run coverage.  Coverage must be triggered manually with
  `make TARGET=opt_cov opt cov ARCHV=81`.

### Future coverage tasks

- [x] Redirect `merge_cov.py` diagnostics to stderr (done 2026-04-28: added `file=sys.stderr` to three print() calls in set_offset/add_data)
- [x] Add `.DELETE_ON_ERROR` or empty-output guard to `cov.txt` target (done 2026-04-28: moved to Makefile.coverage as `$(INSTALLPATH)/cov.txt` with `.DELETE_ON_ERROR`)
- [x] File-based DAG for coverage pipeline (done 2026-04-28):
      - `cov.txt`/`cov.rpt` moved to `$(INSTALLPATH)/` (no longer written to source root)
      - `COV_FILES` + `$(TEST_BUILD_ROOT)/%/cov.txt` pattern rule in `Makefile.coverage`
      - `h2_cov` drives `$(INSTALLPATH)/cov.rpt` directly — no `prepare`/`all` sub-makes
      - `T ?= opt` + `export T` fix: fallback rules now use `$(BUILD_TYPE)` not `$(TARGET)`, so
        `TARGET=opt_cov` no longer tries `make opt_cov` (invalid target)
      - `cov_rpt_tool` now accepts a file path argument (was hardcoded to `cov.txt`)
      - Incremental: second `make TARGET=opt_cov cov` is a true no-op
- [x] v81/opt coverage results (2026-04-28, after pipeline fixes): 353 tracked functions
      (vs 220 before — old pipeline was corrupted by stdout diagnostics)
      - **45 functions at 0%**, **249 functions at 100%**, ~59 partial
- [ ] Investigate 0% functions: which are genuine test gaps vs. intentionally untestable?
      Initial analysis of the 45 zero-coverage functions:
      - **~5 false negatives** (`H2K_tree_remove`, `H2K_id_from_context`, `H2K_mem_tlb_read`,
        `H2K_mem_tlboc`, `H2K_ring_next`): `static inline` wrappers — inlined everywhere,
        no standalone symbol. E.g. `tree_remove` calls `tree_remove_key` which IS 100%.
      - **~8 asm functions**: `H2K_fatal_crash`, `H2K_handle_nmi`, `H2K_handle_rsvd`,
        `H2K_popup_cancel`, `H2K_ext_context_restore`, timer asm (`get_freq`,
        `get_resolution`, `delta_timeout`). Hard-to-reach (crash/NMI paths) or
        timer VM trap handlers not called by current tests.
      - **Confirmed genuine gap**: `H2K_timer_get_freq`, `H2K_timer_get_resolution`,
        `H2K_timer_delta_timeout` — these are registered timer vmtrap dispatch
        functions (freq/resolution/delta queries). The timer tests only use
        `h2_time_get_time()`/`h2_time_set_timeout()` and never invoke these traps.
      - **~32 other C functions**: mix of further gaps (`H2K_intpool_cancel`,
        `H2K_varadix_update/mktrans/walk`, `H2K_trap_hwconfig_*`) and
        intentionally hard paths (HVX coprocessor context, TLB management ops)
      Priority candidates for new tests: timer vmtraps (freq/resolution/delta),
      `futex_cancel`, `vmop_boot/free/status`, `intpool_cancel`, `varadix_update`
- [ ] Run coverage for all four ARCHVs and compare function lists
- [ ] Consider integrating a coverage summary into the unified `testall` report

## Architecture Versions (ARCHV)
- v3/v4/v5: older versions, different interrupt mask conventions.
- v60, v65, v68, v73, v81+: modern versions.  Key differences:
  - v65+: different HVX clock/power control registers.
  - v68+: HMX support, dynamic TLB size from cfg_table, dm0 register.
  - v73+: DMA TLB, vwctrl register.
  - v81: current primary target.

## Clarifications from Session

### Angel (semihosting)
Angel is a semihosting interface based on the ARM semihosting interface, used for
I/O and debug support primarily in simulation.  Preference is to remove it entirely,
but the simulator-based interface will likely have a long life.
`NULL_ANGEL_TRAP` disables the angel TLB mapping for builds that don't need it.

### Popup Threads
Popup threads are threads that block waiting for a specific hardware interrupt.
When that interrupt fires, the thread unblocks and begins executing immediately
with very low overhead.  They are not dynamically created; they are pre-existing
threads that stay blocked until their interrupt arrives.  The effect is similar
to a dynamically-spawned interrupt handler thread but with much lower latency.

### HVX / HMX / HLX Coprocessors
Currently H2 uses a reservation/fixed-allocation model: applications reserve the
coprocessor they need rather than the kernel dynamically managing it.
`H2K_ext_context` exists for future preemption support but is not exercised in
the current test workloads.
NOTE: discuss HVX context switching theory if it comes up later.

### STLB (Software TLB)
A software cache that accelerates TLB miss handling.  Entries are stored in
hardware TLB format so a hit can be inserted into the hardware TLB very quickly
without reformatting.  It is NOT a mirror of the hardware TLB; it is a separate,
software-managed structure.

### Booter
Booter is built into the same binary as the H2 kernel.  At boot, the kernel
creates booter as a VM with full privileges.  Booter then uses file I/O and
H2 trap calls to: create a new VM, load an ELF into it, start it, and wait for
it to exit.  This gives a clean unprivileged execution environment for tests.

### Trap0 / Trap1 Service Numbers
- `libs/h2/vmtraps` -- user/guest-facing trap definitions.
- `kernel/event/trap` -- kernel-side trap dispatch.
- TODO: improve docs and constants in these areas (future task).

### Angel (long-term direction)
Preference is to remove Angel entirely; however the simulator-based Angel
interface will likely have a long life.  `NULL_ANGEL_TRAP` disables the angel
page mapping for builds that don't need it.

### H2K_switch(me, NULL) -- "Go to Sleep"
Passing NULL as the new thread to H2K_switch means "switch to idle" (no runnable
thread).  The inline asm workaround in dosched.ref.c is due to a compiler bug in
toolchain 8.7+ that prevents a clean direct call.

### PKW (Package Wrapper)
Qualcomm-internal tool wrapper infrastructure.  A symlink named after each tool
(e.g., `hexagon-clang`) points to PKW, which reads environment variables or config
files to locate the correct tool installation.  Set `USE_PKW=1` in the build to
enable it.

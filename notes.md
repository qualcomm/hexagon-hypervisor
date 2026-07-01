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

## dlopen / shared library printf investigation — DONE (2026-06-10)

This work spans two sessions (sys_mmap commit 328e12ca, then this session).
The goal: get shared libraries to call libc functions (like printf) that are statically
linked into the main binary.  **This now works end-to-end.**

### Problem
`french.so` (compiled `-fPIC -shared`) has `printf` as an undefined PLT symbol.
At `dlopen` time, the rtld needs to find `printf` from a loaded object.  In a standard
Linux environment the main binary exports symbols via `.dynsym`, but the h2 environment
uses a statically-linked main binary with no `.dynsym` by default.

### dlinit — what it is
`dlinit(argc, argv)` is NOT standard POSIX.  It is Hexagon/BSD-rtld-specific.
It initializes the library-mode rtld.  The `argv` list names "builtin" shared libraries
that are pre-loaded as part of the main binary.  For the builtins mechanism to work, the
main binary must export those symbols via `.dynsym`.
`-rdynamic`, `-Wl,--export-dynamic`: silently ignored by the Hexagon LLD for static
binaries — no `.dynsym` is created.

### Solution: --force-dynamic + hexagon-clang++ for linking

`-Wl,--force-dynamic -Wl,-E` forces the Hexagon LLD to create `.dynsym` with all global
symbols exported.  This makes `dlinit(5, builtin)` find `printf` and C++ runtime symbols
successfully.

**Problem with --force-dynamic**: the binary acquires `.ctors`/`.dtors`/`_init` sections.
The h2 startup calls `_init` before `main()`, which iterates `.ctors` and calls a
constructor (likely `__register_frame_info_base` at ~0xe088) that crashes silently.
The VM runs ~48k instructions and produces no output.

**Fix**: link with `hexagon-clang++` instead of `hexagon-clang`.  The C++ driver
properly links the full C++ runtime (libc++, libc++abi) which provides working
implementations of `__register_frame_info_base` and related frame-registration
functions.  With `hexagon-clang++` the ctors run without crashing and `main()` is
reached normally.  No `noinit.c` stub needed.

**Note on earlier workaround**: a `noinit.c` providing no-op `_init`/`_fini` also worked
for C-only use, but it silently suppresses ALL main-binary constructors.  The
`hexagon-clang++` fix is correct — it lets real C++ ctors run.

### C++ constructor/destructor support (2026-06-10)

Both main-binary and shared-library C++ static constructors and destructors work.

**Main binary ctors**: link with `hexagon-clang++`; add `.cpp` objects to the link.
Hexagon compilers use `.ctors`/`.dtors` (not `.init_array`).  The h2 startup calls
`_init` which iterates `.ctors`.

**Shared library ctors**: `hexagon-clang++` builds a fully-functional shared library with
`DT_INIT` pointing to an `_init` that iterates `.ctors`.  The rtld calls it on `dlopen`.

**NEEDED resolution for C++ .so files**: `cppso.so` built with `hexagon-clang++` has
NEEDED entries for `libc++.so.1`, `libc++abi.so.1`, and `libgcc.so`.  These are
satisfied via dlinit builtins (the rtld resolves their symbols from the main binary's
exported symbol table rather than loading them as files).  Key requirements:
- Add `"libc++.so.1"` and `"libc++abi.so.1"` to the dlinit builtins list.
- Link `libc_eh.a` with `--whole-archive` into the main binary — it provides
  `_Get_eh_data` (needed by libc++abi) which is not pulled in by `-moslib=h2`.
  Without `--whole-archive` the linker drops it since no main-binary code calls it.
- `libhexagon.so` is NOT needed; `libc_eh.a` covers all the EH symbols.

**Separate CC/CXX compilation**: `.c` files compile with `hexagon-clang`; `.cpp` files
with `hexagon-clang++`; final link uses `hexagon-clang++`.  This avoids C++ strict-typing
errors on the C files while still using the C++ driver for linking.

**crtbeginS.o / crtendS.o note**: these exist under `.../v73/G0/pic/` but use
non-PIC relocations and cannot be linked into a `-shared` binary.  The `hexagon-clang++`
driver handles the shared library startup correctly without them when invoked without
`-nostartfiles`.

### Bug fixed: fake fstat st_ino collision

`dltest/fstat.c` had `buf->st_ino = fd`.  The rtld uses `st_dev`+`st_ino` to detect
already-loaded libraries.  If two `.so` files are opened on the same fd number (e.g.,
both on fd 3 at different times), both get identical `st_ino`, and the rtld returns the
cached handle for the first library instead of loading the second.
**Fix**: use a static counter for `st_ino` so every `fstat` call returns a unique inode.

### Bug fixed earlier: booter elf_get_specials for dynamic binaries
`libs/h2_compat/elf/h2_elf.ref.c` `elf_get_specials()` picked up `.dynstr` instead of
`.strtab` for dynamic binaries (`.dynstr` comes first in section order; both are
`SHT_STRTAB`).  Fix: use `sh_link` to look up the
correct string table for the symbol table section.

### Other findings during investigation

- **`--dynamic-linker=` (empty)**: has no effect; the booter ignores `PT_INTERP`.
- **No GOT/PLT**: lld does not create GOT/PLT for a fully-static `--force-dynamic`
  binary.  All symbol references are pre-resolved at link time.
- **libdl.a must match the target arch**: using v68 `libdl.a` in a v73 binary causes
  `dlinit` to hang (jump-to-self).  Always use the matching arch version.
- **Do NOT add `-G0` to CC**: with `-moslib=h2`, adding `-G0` silently breaks output
  (printf produces nothing).  The h2 startup manages GP correctly without it.
- **rtld requires absolute paths**: `dlopen("./foo.so")` fails with "abs pathname
  required".  `dlopen("foo.so")` (bare name) works if `dlinit` listed it as a builtin.
- **Real libc.so cannot be dlopen'd directly**: the prebuilt `libc.so` has its own
  `_init` that crashes in the h2/booter environment (jump-to-self).  The builtins
  mechanism avoids loading it as a file.
- **Toolchain reference example**: `dltest/example/` contains the canonical standalone
  (non-h2) dlopen example from the toolchain, adapted for v73 + archsim.  It uses
  `--whole-archive libc.a` + `--force-dynamic -E` + fake fstat.  Confirmed working.
  The key structural difference from the h2 approach: standalone uses
  `crt0_standalone.o + libstandalone.a`; h2 uses `-moslib=h2`.

### Working configuration (dltest/)

C and C++ objects compiled separately, linked with `hexagon-clang++`:

```makefile
CC  = hexagon-clang   -mv73
CXX = hexagon-clang++ -mv73

# C++ .so — full clang++ build, DT_INIT set automatically
cppso.so: cppso.cpp
    $(CXX) -fPIC -Wl,-Bsymbolic -shared -o cppso.so cppso.cpp

# Main binary — link with CXX to get correct C++ runtime
libdltest: libdltest.o fstat.o sys_mmap.o cpptest.o
    $(CXX) -moslib=h2 -Wl,-Bsymbolic -Wl,--export-dynamic -Wl,--force-dynamic \
        -Wl,--dynamic-linker= -Wl,--allow-multiple-definition \
        -L.../artifacts/v73/opt/install/lib \
        -o libdltest ... $(LIBDL) \
        -Wl,--whole-archive $(LIBC_EH) -Wl,--no-whole-archive
```

dlinit builtins: `{"libgcc.so", "libc.so", "libstdc++.so", "libc++.so.1", "libc++abi.so.1"}`

Run: `PKW_arch=v73_stable archsim --quiet .../booter libdltest`

Output:
```
C++ ctor: main binary       ← static global in cpptest.cpp, fires before main()
faux @ 28150
Hello from library!
faux returned: Barre.
C++ ctor: cppso.so          ← static global in cppso.cpp, fires at dlopen() time
cppfunc returned: Hello from C++ shared library!
C++ dtor: cppso.so          ← fires at exit / dlclose
C++ dtor: main binary       ← fires at exit
```

### dltest/ directory layout
- `dltest/french.c` — C shared lib: `faux()` calls `printf`, returns `"Barre."`
- `dltest/cppso.cpp` — C++ shared lib: static `SoCtor` global (ctor/dtor), `cppfunc()`
- `dltest/libdltest.c` — main: `dlinit(5, builtins)`, dlopen/dlsym both .so files
- `dltest/cpptest.cpp` — C++ static global in main binary to test main-binary ctors
- `dltest/fstat.c` — fake `fstat`: uses `sys_flen` + unique `st_ino` counter
- `dltest/sys_mmap.c` — mmap via angel (also in libs/syscall/angel)
- `dltest/Makefile` — separate CC/CXX compile, CXX link, v73 libdl.a + libc_eh.a
- `dltest/example/` — standalone (non-h2) reference: works under both hexagon-sim
  and archsim; uses `crt0_standalone.o + libstandalone.a + --whole-archive libc.a`

### Next tasks
- Integrate dlopen support into the main test infrastructure
- Consider promoting `sys_mmap` and fake `fstat` to a proper h2 library

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

## Thread-kill API design (2026-06-24) — VMOP_KILL_THREAD implemented

Wire up `H2K_VMSTATUS_KILL` (already defined, already handled in
`kernel/vm/vmwork/vmwork.ref.c:18`) into a cross-thread "kill" mechanism so one
thread can terminate another.  Also a stepping stone for a future "kill VM"
operation.

### Existing infrastructure
- `H2K_VMSTATUS_KILL` (bit 1 of `atomic_status_word`, `kernel/util/vmdefs/vmdefs.h:33`).
- `H2K_vm_do_work` already checks the bit and calls `H2K_thread_stop` if set.
- `H2K_vm_do_work` runs on: context switch (`switch.ref.S:285`), IPI delivery
  (`vmipi.ref.c:36`), futex wake (`futex_classic`, `futex_pi`), vmwait
  (`vmfuncs.ref.c:115`).
- `H2K_thread_stop` does full teardown: timer cancel, runlist remove, asid dec,
  context clear, parent signal via `H2K_VM_CHILDINT`.
- `H2K_vm_int_deliver_locked` (`vmint.ref.c:28`) is the canonical "wake from
  any state" routine.  Handles VMWAIT / RUNNING / INTBLOCKED / BLOCKED / READY
  / DEAD.  Has an IE-guard that we don't want for kill.

### Design decisions
1. **API surface**: new `VMOP_KILL_THREAD` appended to `vmop_t` in
   `libs/h2/common/h2_common_vm.h`, dispatch entry added to
   `kernel/vm/vmop/vmop.ref.c`.  Arg layout: `(unused0, id, ...)`.
   Returns 0 / -1.
2. **Permissions**: same as `H2K_vmop_status` — caller must be in the target's
   VM (self) or be the target's parent VM.
3. **IE override**: kill ignores guest IE; it's forceful.
4. **State handling**: duplicate the state-switch logic from
   `H2K_vm_int_deliver_locked` (we discussed refactoring into a shared helper
   but chose to duplicate to keep the asm version untouched).  Add a top-of-file
   FIXME noting that a future "thread cancellation" primitive could unify the
   two state machines.
5. **DEAD = success**.  Also "vmblock pointer is NULL" (whole VM gone) = success.
   Malformed ID (out-of-range vmidx/cpuidx) returns -1 naturally; deferred.
6. **Exit status**: fixed sentinel `0xd1edd1ed` (kill-by-other), distinct from
   `0xd1eed1ee` (self-kill via vmwork).  Caller-supplied exit codes deferred
   until needed by `kill_child`.
7. **Self-kill**: target == me goes through the same path; the RUNNING case
   sends a self-IPI which fires on trap return and runs `vm_do_work`.  No
   special-case in the kill helper.
8. **Concurrent kills / kill racing with voluntary stop**: BKL serializes;
   bit-set is atomic; whoever loses finds DEAD and returns success.

### Implementation steps (one per session per CLAUDE.md guidance)
1. **DONE 2026-06-24**: `H2K_vmop_kill_thread` added to
   `kernel/vm/vmop/vmop.ref.c`; `VMOP_KILL_THREAD` enum entry in
   `libs/h2/common/h2_common_vm.h`; declaration in `kernel/vm/vmop/vmop.h`;
   spec entry in `kernel/vm/vmop/vmop.spec`.  Builds clean on opt and ref
   variants (v81).  The implementation kept sentinel `0xd1eed1ee` — the
   target's exit status is still whatever `vmwork.ref.c` passes to
   `H2K_thread_stop`, since the natural-handling design means a separate
   "kill-by-other" sentinel needs either a second bit or a stashed status word.
   Deferred unless `VMOP_KILL_CHILD` actually needs it.
2. **DONE 2026-06-24**: Test added at `kernel/vm/vmop/test/test.c` (replaces
   the placeholder "Tested by thread/create/test"). Mocks
   `H2K_vm_ipi_send_withlock`, `H2K_popup_cancel`, `H2K_futex_cancel`; uses the
   real `H2K_ready_append` (it just touches the readylist globals). Covers
   permissions (me==NULL, VM-gone, unrelated VM, OOB cpuidx), DEAD short-
   circuit, all five non-DEAD states (VMWAIT/RUNNING/INTBLOCKED/BLOCKED/READY),
   IE-clear variants for the kill-ignores-IE invariant, and self-kill. Passes
   on v81/opt as part of the 99-test suite.

   Side cleanup: the `target_id.vmidx >= H2K_ID_MAX_VMS` check in
   `H2K_vmop_kill_thread` was dead code (the field is a 6-bit bitfield, so it
   can only hold 0..63 == 0..H2K_ID_MAX_VMS-1). Removed.
3. **DONE 2026-06-24**: `H2K_vmop_kill_child` added. Refactor was tiny: pulled
   the BKL-locked body of `H2K_vmop_kill_thread` into static
   `kill_thread_locked(vmblock, target)` and called it once from kill_thread,
   in a loop from kill_child. Permission is **parent-only** (caller's
   `id.vmidx == vmblock->parent.vmidx`); the calling VM cannot use this op to
   kill its own threads (use `VMOP_KILL_THREAD` for self). VM-already-gone =
   success, matching kill_thread. We let `H2K_thread_stop` write
   `vmblock->status` naturally as each vcpu drains, rather than stamping a
   "being killed" status up-front. Coverage report shows zero uncovered lines
   across all three functions; full v81/opt suite still 99/99.
4. **(Future)** refactor `H2K_vm_int_deliver_locked` and `kill_thread_locked`
   to share a common "wake-from-any-state" primitive; update the asm.

## VMOP_KILL_VM + pthread exit tests (2026-06-25) — IN PROGRESS

### What was done this session

**1. Ported pthread test suite from `kernel_thread_stop_reap_blocked` PR**

`git checkout origin/kernel_thread_stop_reap_blocked -- libs/posix/pthread/test_h2/ makefile scripts/Makefile.coverage scripts/Makefile.inc.test scripts/testlist.v61`

36 new tests in `libs/posix/pthread/test_h2/`. Three categories:
- Basic pthread tests (join, mutex, cond, barrier, rwlock, sem, TLS, attrs) — `join_basic`, `mutex_recursive`, etc.
- `stuck_in_*` tests — main returns while worker threads are blocked in various syscalls. Pass only if the VM-exit reaper kills stuck threads.
- `exit1_*` tests — main calls `exit(1)` with stuck workers. Pass if VM exits on non-zero status.

Also ported: `makefile` change (leading `-` on test rules so one failure doesn't stop the run), `scripts/Makefile.coverage` symlink filtering fix, `scripts/Makefile.inc.test` `.PHONY` additions, `scripts/testlist.v61` registration of all 36 tests.

**2. Baseline test results (pre-fix, v81/opt)**

- **22 basic tests**: all PASS
- **`stuck_in_rwlock_rd/wr`**: PASS (rwlock is ENOSYS stub, threads don't actually block)
- **`stuck_in_barrier`**: PASS (barrier uses INTBLOCKED state which the "all-blocked" heuristic in `H2K_thread_stop` handles)
- **`exit1_main_mutex`, `exit1_main_cond_wait`, `exit1_worker_cond_wait`, `exit1_detached_worker_with_stuck`**: PASS (non-zero exit status triggers parent signal in existing `H2K_thread_stop`)
- **`stuck_in_mutex`, `stuck_in_join`, `stuck_in_cond_wait`, `stuck_in_sem_wait`, `stuck_in_sem_timedwait`, `stuck_in_cond_timedwait`, `stuck_in_pthread_exit_joined`**: **HANG** (ulimit-killed, exit 137) — these 7 are the target
- **`neg_cond_wait_no_mutex`**: FAIL with "NO PASS/FAIL/ERROR in log" (probes UB, simulator crashes before main finishes)

**3. VMOP_KILL_CHILD → VMOP_KILL_VM rename + self-kill**

Goal: let `sys_exit` call `VMOP_KILL_VM(SELF)` to reap stuck sibling threads before the calling thread exits.

Files changed:
- `libs/h2/common/h2_common_vm.h`: renamed `VMOP_KILL_CHILD` → `VMOP_KILL_VM`; added `#define VMOP_KILL_VM_SELF 0` (sentinel, consistent with `VMOP_STATUS_VMIDX_SELF 0`)
- `kernel/vm/vmop/vmop.h`: renamed `H2K_vmop_kill_child` → `H2K_vmop_kill_vm`
- `kernel/vm/vmop/vmop.ref.c`: renamed function + dispatch table entry; added SELF sentinel handling (`if (vm == VMOP_KILL_VM_SELF) vmblock = me->vmblock`); expanded permission check to allow self-VM (`me->id.vmidx == vmblock->vmidx`) OR parent-of-child (`me->id.vmidx == vmblock->parent.vmidx`)
- `libs/h2/vm/h2_vm.h`: added `int h2_vmkill(unsigned int vm)` declaration
- `libs/h2/vm/h2_vm_imp.ref.c`: added `int h2_vmkill(unsigned int vm) { return h2_vmop_trap(VMOP_KILL_VM, vm, 0, 0, 0, 0); }`
- `libs/syscall/angel/src/sys_exit.c`: added `#include <h2_vm.h>`, `#include <h2_common_vm.h>`, `#include <stdio.h>`; added `h2_vmkill(VMOP_KILL_VM_SELF)` call + debug printf at top of `sys_exit`
- `kernel/vm/vmop/test/test.c`: updated test P (self-kill now expected to SUCCEED, caller gets KILL set); updated test comment header; added test T (VMOP_KILL_VM_SELF sentinel with two live threads); renamed all `H2K_vmop_kill_child` → `H2K_vmop_kill_vm`

**4. Build system gotcha — important for future sessions**

The libs build has a two-phase install/compile pattern:
- `libs/h2/common/h2_common_vm.h` is a source file but ALSO gets installed to `artifacts/v81/opt/install/include/h2_common_vm.h` during `make build`
- `libs/h2/vm/h2_vm_imp.ref.c` compiles with `-I$(INSTALLPATH)/include` — it reads from the **installed** header, not the source file
- If the installed header is stale (not yet installed after a source change), `h2_vm_imp.ref.c` will fail to compile with "use of undeclared identifier 'VMOP_KILL_VM'"
- Workaround: `cp libs/h2/common/h2_common_vm.h artifacts/v81/opt/install/include/` before rebuilding
- **Permanent fix**: delete all of `artifacts/` and rebuild from scratch — this is what we did at the end of the session

**5. Current state (end of session)**

- `rm -rf artifacts/` + `make ARCHV=81 TARGET=opt build` completed successfully (exit 0)
- `h2_vmkill` at offset 0x30 confirmed in `artifacts/v81/opt/install/lib/libh2.a`
- Debug strings "sys_exit(%d): killing vm" and "sys_exit: vm killed, exiting thread" confirmed in `sys_exit.o`
- **Tests have NOT been run yet on the clean build**

### Next session: run tests and interpret results

```
make ARCHV=81 TARGET=opt test_variant
```

Then check results:
```
grep "^exit status" artifacts/v81/opt/build/tests/./libs/posix/pthread/test_h2/stuck_in_mutex/make.log
```

For each of the 7 previously-hanging tests (`stuck_in_mutex`, `stuck_in_join`, `stuck_in_cond_wait`, `stuck_in_sem_wait`, `stuck_in_sem_timedwait`, `stuck_in_cond_timedwait`, `stuck_in_pthread_exit_joined`) — look for `exit status == 0`.

The debug prints in `sys_exit` will show in `test.log` if `sys_exit` is being called and whether `h2_vmkill` returns before the hang.

If tests still hang: look at the output after "sys_exit: vm killed, exiting thread" — if that line doesn't appear at all, `sys_exit` is not being reached (check if C runtime calls `_exit` instead of `sys_exit`). If it appears but simulator keeps running, the kill happened but `H2K_thread_stop` on the main thread isn't signaling the parent (check `vmblock->num_cpus` race).

### Key mechanism (how the fix should work)

1. `main()` returns 0 → C runtime calls `sys_exit(0)`
2. `sys_exit` calls `h2_vmkill(0)` → `VMOP_KILL_VM` with SELF sentinel
3. Kernel: `H2K_vmop_kill_vm(0, 0=SELF, ...)` → loops over vmblock's contexts → calls `kill_thread_locked` on each:
   - BLOCKED threads (waiting on futex): `H2K_futex_cancel` + `H2K_ready_append` + sets KILL+VMWORK
   - RUNNING thread (self): sets KILL+VMWORK + sends self-IPI
4. `h2_vmkill` returns to `sys_exit`
5. `sys_exit` calls `pthread_exit(0)` (via hook, main thread is detached so no ack-wait)
6. `pthread_exit` → `H2_TRAP_THREAD_STOP` → `H2K_thread_stop(0, main)`
7. `H2K_thread_stop`: `num_cpus--`, `vmblock->status=0`. If `num_cpus > 0` and `status == 0`, no parent signal yet. Calls `H2K_dosched`.
8. Scheduler runs READY sibling threads. Each one: context switch → `switch.v4opt.S` sees VMWORK → calls `H2K_vm_do_work` → sees KILL → `H2K_thread_stop(0xd1eed1ee, sibling)`
9. `H2K_thread_stop(0xd1eed1ee != 0, ...)` → signals parent via `H2K_VM_CHILDINT`
10. Booter wakes, sees VM exited → simulator exits

### Remove debug prints before committing

The `printf` calls added to `sys_exit.c` are temporary debug aids. Remove them before the final commit:

```c
// Remove these two lines:
printf("sys_exit(%d): killing vm\n", (int)status);
printf("sys_exit: vm killed, exiting thread\n");
// And remove #include <stdio.h>
```

## VMOP_KILL_VM working end-to-end (2026-06-25 evening) — TESTS PASSING

### TL;DR

`stuck_in_mutex` and 3 other previously-hanging pthread tests now PASS.  The
hang was a BKL re-acquire deadlock that the previous session's notes did not
identify.  Root cause fixed by splitting `H2K_thread_stop` and `H2K_vm_do_work`
into wrapper / `_withlock` variants.  `h2_vmkill` now takes a `status` parameter,
and the kill loop skips the caller so it can unwind through its own exit path.

### The actual bug (previous session got close but stopped)

Previous session's debug printfs in `sys_exit.c` used `printf` and never
appeared in the test log — they almost certainly would have, because `sys_exit`
*was* being reached.  The reason they didn't show up: after `exit()` runs
stdio cleanup, the printf buffers are torn down, so anything `sys_exit` writes
through stdio is lost.  Using `sys_writec`/`SYS_WRITECREG` (no buffering) made
the trace visible.  Also `sys_write0` printed pointer bytes instead of the
string in the v81 sim — `__sys_write_mode__` defaults to 0, which apparently
interprets the trap argument as a value rather than a pointer.  Avoid
`sys_write0`; loop over chars with `SYS_WRITECREG`.

The deadlock:

1. `main` returns 0 → `sys_exit(0)` → `h2_vmkill(SELF)` reaps siblings + self.
2. Main exits via self-IPI → `H2K_vm_do_work` → `H2K_thread_stop` → posts
   CHILDINT, calls `H2K_dosched` while still holding BKL (per spec, dosched
   requires BKL held; BKL is released later by `H2K_check_sanity_unlock`).
3. `dosched` picks a sibling (KILL+VMWORK), jumps to `H2K_switch`.
4. `switch.v4opt.S:383` sees VMWORK and calls `H2K_vm_do_work` **with BKL
   still held**.
5. `H2K_vm_do_work` sees KILL, calls `H2K_thread_stop` → which starts with
   `BKL_LOCK` → **deadlock** (k0lock on already-held lock).

Pre-commit-7b5abe11 `H2K_vm_do_work` only called `H2K_vm_check_interrupts`
(BKL-fluid), so it was safe with or without BKL held.  Commit 7b5abe11 added
the KILL → `thread_stop` branch and broke the contract.  The `dosched` and
`stop` specs were unambiguous about BKL state but `vmwork.spec` was silent —
that's now fixed.

### Fix

**1. Split lock-aware variants** (matches existing `_withlock`/`_locked`
convention used elsewhere in the kernel):

- `kernel/thread/stop/stop.{c,h,spec}` — `H2K_thread_stop_withlock` (assumes
  BKL held); `H2K_thread_stop` is a wrapper that does `BKL_LOCK` then calls it.
- `kernel/vm/vmwork/vmwork.{c,h,spec}` — `H2K_vm_do_work_withlock` likewise.

**2. Update BKL-held callers to call `_withlock` directly:**
- `kernel/sched/switch/switch.v4opt.S:383` → `H2K_vm_do_work_withlock`
- `kernel/sched/switch/switch.ref.S:285` → same
- `kernel/vm/vmfuncs/vmfuncs.ref.c:115` → same
- `kernel/vm/vmfuncs/vmfuncs.v4opt.S:167` → same

Other callers (futex_classic, futex_pi, vmipi) keep using the wrapper — they
explicitly unlock before calling and the wrapper re-acquiring is harmless.

**3. Status parameter and skip-self for `h2_vmkill`:**

- `h2_vmkill(unsigned int vm, int status)` — user-side; 2nd arg sets
  `vmblock->status` before the kill loop.
- `H2K_vmop_kill_vm(..., u32_t vm, u32_t status, ...)` — kernel side.
- Skip self in the kill loop (caller continues, unwinds via its own exit path).
- `H2K_vm_do_work_withlock`'s KILL branch passes `me->vmblock->status` to
  `H2K_thread_stop_withlock` (instead of the old hardcoded `0xd1eed1ee`).

The user accepted the asymmetry: "kill_vm of self skips one; kill_vm of a
child kills all."  Reasoning: caller-that-also-wants-to-die can explicitly
call `h2_thread_stop` after `h2_vmkill`.  The hardcoded `0xd1eed1ee` marker
is no longer used — if you want it, pass it as the `status` argument.

### Two important build/diagnostic gotchas (do not re-burn time on these)

**Kernel logging is off by default in opt builds.**  `H2K_log()` and
`H2K_log_string()` expand to nothing unless the kernel is compiled with
`-DH2K_LOGBUF`.  That macro is gated by `LOGBUF` on the make command line:

    make ARCHV=81 TARGET=opt LOGBUF=1 build

Without `LOGBUF=1`, kernel-side log calls are silent and you'll waste time
wondering why your prints aren't showing up.

**libs incremental build is broken when sources change but timestamps look
ok.**  Editing `libs/syscall/angel/src/sys_exit.c` does not always cause
`sys_exit.o` and `libh2.a` to rebuild — even with newer source timestamps.
Workaround that's reliable:

    find artifacts/v81/opt -name 'sys_exit.o' -delete
    find artifacts/v81/opt -name 'libh2.a' -delete
    find artifacts/v81/opt -name 'manifest*' -delete
    make ARCHV=81 TARGET=opt LOGBUF=1 build

ALWAYS confirm a rebuild happened by checking `strings libh2.a` for a string
you just edited.  This bug bit the previous session multiple times and made
them think their changes weren't working when in fact the binary was stale.

### Fast iteration: run one test in ~30 seconds

`ULIMIT` defaults to 600 (CPU seconds) — way too long for debug iterations.
Override per-run:

    H2DIR=$(pwd) INSTALLPATH=$(pwd)/artifacts/v81/opt/install \
      make -f scripts/Makefile.coverage ARCHV=81 TARGET=opt ULIMIT=30 \
        $(pwd)/artifacts/v81/opt/build/tests/<test-path>/results.txt

Hanging tests are killed in ~30s of CPU time.  Pass = `results.txt` contains
`TEST PASSED` and `make.log` has `exit status == 0`.

### What's left (next session start here)

**Cleanup before committing:**

1. Remove debug instrumentation from kernel:
   - `kernel/vm/vmop/vmop.ref.c`: H2K_log calls in `H2K_vmop_kill_vm` and
     `kill_thread_locked`.
   - `kernel/vm/vmwork/vmwork.ref.c`: H2K_log calls in `H2K_vm_do_work*`.
   - `kernel/thread/stop/stop.ref.c`: H2K_log calls in `H2K_thread_stop*`.
   - Drop the `#include <log.h>` lines added to those files (unused after).
2. Remove `dbg_write` helper and its calls from `libs/syscall/angel/src/sys_exit.c`.
3. The previous session's `printf` + `#include <stdio.h>` are already replaced
   by `dbg_write` and should be gone after step 2.

**Verification still to do:**

1. Run the remaining stuck_in_* tests we haven't tried yet:
   `stuck_in_sem_timedwait`, `stuck_in_cond_timedwait`,
   `stuck_in_pthread_exit_joined`.  Expected: PASS.
2. Run the full `make ARCHV=81 TARGET=opt test_variant` to make sure we
   didn't break anything that was passing.  Watch especially for tests that
   exercise BKL (futex, scheduler) since we touched those paths.
3. Confirm `neg_cond_wait_no_mutex` still behaves the same way it did
   before (it was already FAILing with "NO PASS/FAIL/ERROR in log" — that's
   a different issue, not regressed by our changes).

**Design follow-ups (optional, defer):**

- The `_withlock` naming was extended to `H2K_thread_stop_withlock` and
  `H2K_vm_do_work_withlock`; spec files updated.  But the project also uses
  `_locked` (e.g. `H2K_vm_cpuint_post_locked`).  Conventions split between
  `_withlock` and `_locked` historically — not worth normalizing in this PR.
- `VMOP_KILL_VM_SELF == 0` sentinel sharing space with vmidx 0 (booter).
  Cannot ever target the booter VM through this vmop.  Booter shouldn't be
  killable anyway, so deferred.
- The previous session's whole "vmkill(SELF) kills the caller" model proved
  wrong — skip-self is cleaner.  The old test T was rewritten to verify
  skip-self semantics + status stamping.

## VMOP_KILL_VM session 2 (2026-06-25 night) — 134/136 PASSING

### TL;DR (continuing the morning session)

Started this session at 67/136 because of test.log corruption.  After a
full diagnosis + cleanup pass we're at 134/136 v81/opt.  The two remaining
fails are isolated and well-understood (see "Next session" below).

### What was wrong with the morning's 67/136

Two layered bugs hid the real test results:

1. **`dbg_write` in `sys_exit` polluted standalone test logs with binary
   characters.**  The previous session added `dbg_write` using
   `SYS_WRITECREG` to debug the deadlock.  Even after cleanup, our new
   `h2_vmkill` call in `sys_exit` was being issued from STANDALONE kernel
   tests (kernel/*/test/*) which run in monitor mode with no h2 hypervisor
   underneath.  The TRAP1 (`h2_vmop_trap`) goes to a simulator that doesn't
   recognize the SWI — archsim emits `I don't know that swi request: 0x20148`
   and writes 3 stray bytes (`L\300\0`) to stdout.  The trailing NUL flipped
   `grep` into "binary file matches" mode, so `results.txt` was a 0-byte file
   for every standalone test even though they all printed "TEST PASSED".

2. **Cleanup of session 1's debug instrumentation** was not done.  The
   morning's first runs still had the H2K_log calls in
   `vmop.ref.c`/`vmwork.ref.c`/`stop.ref.c` and the `dbg_write` helper in
   `sys_exit.c`.  All removed this session.

### Fix #1: standalone-aware sys_exit

`sys_exit` now gates the `h2_vmkill` + pthread_exit hook on the existing
`__h2_thread_stop_hook__ != 0xfffffff0` sentinel.  Standalone builds
(linked with `-Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0` in
`scripts/Makefile.inc.test:174,180`) now go straight to `ANGEL(SYS_EXIT)`,
exactly as they did before VMOP_KILL_VM existed.  Code:

```c
void sys_exit(okay_t status)
{
    /* Sentinel hook (0xfffffff0) means standalone build (no h2 hypervisor
     * underneath, e.g. kernel-level tests).  TRAP1-based h2 calls would
     * go nowhere, so skip the vm reap and the pthread_exit hook. */
    if ((void (*)(int))0xfffffff0 != __h2_thread_stop_hook__) {
        h2_vmkill(VMOP_KILL_VM_SELF, (int)status);
        if (0 == status) {
            __h2_thread_stop_hook__(status);
        }
    }
    ANGEL(SYS_EXIT,status,status);
}
```

Watch the comment: a `kernel/*` glob in a C block comment looks like a
nested `/*` to Werror=comment.  Use `kernel-level` instead.

### Fix #2: split vmwork.ref.c and stop.ref.c

`H2K_vm_do_work` and `H2K_vm_do_work_withlock` lived in the same .o file,
which broke three tests that stub one of those names: archive linking pulls
both symbols and the test's strong def collides with the archive's strong
def.  Same for `H2K_thread_stop`/`H2K_thread_stop_withlock`.

Splits created:
- `kernel/vm/vmwork/vmwork_lock.ref.c` — wrapper `H2K_vm_do_work` only.
- `kernel/vm/vmwork/vmwork.ref.c` — `H2K_vm_do_work_withlock` only.
- `kernel/thread/stop/stop_lock.ref.c` — wrapper `H2K_thread_stop` only.
- `kernel/thread/stop/stop.ref.c` — `H2K_thread_stop_withlock` only.

The wrapper files include `<hw.h>` for `BKL_LOCK`/`BKL_UNLOCK`.  The
`*.ref.c` glob in `kernel/build/make/make.inc.default` picks up the new
files automatically.

### Fix #3: rename test stubs

Tests that mocked the old names now mock the `_withlock` variants because
that's what production calls from BKL-held contexts after the split:
- `kernel/vm/vmwork/test/test.c`:
  - stub `H2K_thread_stop` → `H2K_thread_stop_withlock`
  - call site `H2K_vm_do_work(x)` → `H2K_vm_do_work_withlock(x)`
- `kernel/vm/vmfuncs/test/test.c`:
  - stub `H2K_vm_do_work` → `H2K_vm_do_work_withlock`
  - stub `H2K_thread_stop` → `H2K_thread_stop_withlock`
- `kernel/vm/vmipi/test/test.c`: no change (vmipi.ref.c still calls the
  unlocked wrapper — that's correct, the path comes from futex/vmipi with
  BKL not held).

### Current state (v81/opt, full clean rebuild + test_variant)

- **134 passed, 2 failed**.

Remaining failures:

1. **`kernel/vm/vmfuncs/test`** — hangs silently (`test.log` is empty, exit
   137 from ULIMIT).  `test.elf` links fine; the symbol table shows
   `H2K_vm_do_work_withlock` and `H2K_thread_stop_withlock` resolved to
   the test stubs.  Cause not yet diagnosed.  Hypotheses:
   - The wrapper `H2K_thread_stop` from `stop_lock.ref.o` is pulled in
     (visible at 0x8294) — something references it (likely
     `h2_thread_stop_trap` in libh2.a).  If a path runs it, the wrapper
     does `BKL_LOCK()` + calls `_withlock` (test stub).  But the test
     stub `H2K_thread_stop_withlock` longjmps out — never returning to the
     wrapper's never-was BKL_UNLOCK.  After such a path, BKL is held but
     longjmp escapes, next BKL_LOCK deadlocks.
   - The earlier vmtraps (vmversion, setvec, setie, getie, etc.) don't
     touch BKL or thread_stop — they should run cleanly.  The first
     vmtrap_wait test case (TH_work_ret=1, intno=1, else branch) should
     also be fast.
   - The lack of ANY output (test.log == 50 bytes, only the
     `probably ulimit` line — nothing from boot or the test) suggests the
     hang is very early.  Possibly in startup before `main()` prints
     anything.  Worth checking: does the test link in something new now
     because of the split?  `hexagon-nm test.elf` shows the wrapper
     `H2K_thread_stop` at 0x8294 — that wasn't there before.  Trace what
     references it.
2. **`libs/posix/pthread/test_h2/pthread_workers_return`** — known
   not-fixed by this PR.  Test has main calling `pthread_exit(NULL)` and
   joinable workers that return from start routine; the pthread library
   makes them wait at `pthread_sem_wait_np(&self->joined)` for a join that
   never comes (main is dead).  Needs an "all-threads-blocked-or-dead"
   reaper in the kernel — separate feature from VMOP_KILL_VM (which
   activates only from sys_exit).  Defer; document as expected fail.

### Files modified this session

- `libs/syscall/angel/src/sys_exit.c` — sentinel gate around h2_vmkill+hook,
  comment fixed (`kernel/*` → `kernel-level`).
- `kernel/vm/vmop/vmop.ref.c` — removed all `H2K_log*`, removed `<log.h>`.
- `kernel/vm/vmwork/vmwork.ref.c` — removed wrapper, removed `H2K_log*`,
  removed `<log.h>`.  Now only contains `H2K_vm_do_work_withlock`.
- `kernel/vm/vmwork/vmwork_lock.ref.c` — NEW.  Wrapper only.
- `kernel/thread/stop/stop.ref.c` — removed wrapper, removed `H2K_log*`,
  removed `<log.h>`.  Now only contains `H2K_thread_stop_withlock`.
- `kernel/thread/stop/stop_lock.ref.c` — NEW.  Wrapper only.
- `kernel/vm/vmwork/test/test.c` — stub renamed, call site renamed.
- `kernel/vm/vmfuncs/test/test.c` — two stubs renamed.

### Reproducer for the morning's confusion (in case it happens again)

If you see "tons of standalone tests failing with `FAIL - Empty/No
results.txt`", grep one of the test.logs with `od -c`.  Look for trailing
NUL bytes or other non-printable characters.  If found, search the kernel
and libs for any new direct `ANGEL(...)` / `sys_writec` / `sys_write0`
calls or trap-1 invocations from standalone-test code paths.  The angel
SYS_WRITE0 in particular has the v81-sim bug noted in session 1
(`__sys_write_mode__ == 0` interprets the trap arg as a value, not a
pointer — printing pointer bytes).

`gen_test_results.py` reads `results.txt` content; 0-byte `results.txt`
counts as a failure even though `test.log` may say "TEST PASSED".  The
rule at `Makefile.inc.test:494` uses `grep` which switches to "binary
file matches" mode the moment test.log contains a NUL, and that mode
prints to stderr (in this build of grep) so `results.txt > $@` ends up
empty.

### alive_count fix for pthread_workers_return (2026-06-26) — DONE

Added `static atomic_u32_t alive_count = 1` to `pthread_thread.ref.c`:
- Incremented in `pthread_create` after successful thread creation.
- Decremented in `pthread_exit` (via `h2_atomic_sub32`) before the
  joined-wait.  If the decrement reaches 0, this thread is the last
  live one: call `h2_vmkill(VMOP_KILL_VM_SELF, 0)` to reap any zombies,
  then skip both the `joined` wait and the `exiting/ack` handshake (no
  joiner will ever come).
- `alive_count = 1` covers main; main goes through `pthread_mainthread_setup`
  (not `pthread_create`), so no explicit increment needed.  Main is set
  DETACHED, so it never blocks on joined regardless.
- Uses `h2_atomic_add32`/`h2_atomic_sub32` from `h2_atomic.h` (already
  accessible via `h2if.h`), consistent with `pthread_once.ref.c`'s use of
  `h2_atomic_compare_swap32`.
- Added `#include <h2_common_vm.h>` for `VMOP_KILL_VM_SELF`.

Result: `pthread_workers_return` now passes.  v81/opt: **135/136**.

### Current state (2026-06-26 morning)

- **136 passed, 0 failed** (v81/opt). DONE (2026-06-26).

### vmfuncs/test fix (2026-06-26) — DONE

The actual issue was `stop_lock.ref.o` (wrapper `H2K_thread_stop`) being
pulled in — not `vmwork_lock.ref.o` as predicted.  After the stop split,
something in the vmfuncs test link chain referenced `H2K_thread_stop` by
name.  The wrapper does `BKL_LOCK` → test stub `_withlock` → `longjmp`
escape → BKL never released → deadlock before any output.

Fix: added `H2K_thread_stop` stub to `kernel/vm/vmfuncs/test/test.c` that
forwards to `H2K_thread_stop_withlock` (no locking).  This prevents
`stop_lock.ref.o` from being pulled in at all.

Also recombined `stop_lock.ref.c` back into `stop.ref.c` (deleted
`stop_lock.ref.c`) since the stub in the test now blocks the wrapper from
being pulled from the archive in both `vmfuncs/test` and `vmwork/test`.

### NEXT TASK: run all 4 ARCHVs × {opt, ref} to confirm no regressions, then commit.

### pthread_workers_return — decision direction (2026-06-25 night)

Discussed at end of session.  **Kernel reaper ("all threads blocked → reap
the VM") is OUT** — too easy a footgun (a legitimate wait-for-external-
interrupt VM would silently disappear), and external wakers like vm
interrupts complicate the "no possible waker" check.

Two acceptable directions, to choose tomorrow:

a) **Userspace fix in pthread_exit**: when a thread enters
   `pthread_sem_wait_np(&self->joined)` to wait for a join, it's
   effectively a zombie.  Have `pthread_exit` (before blocking) sweep the
   TCB list and join any already-waiting zombies, OR maintain a
   "live-thread" counter and have the last live thread call `sys_exit(0)`
   instead of blocking on join (sys_exit then triggers VMOP_KILL_VM,
   which reaps the joinables stuck on the semaphore).  The counter
   approach is closer to what glibc's NPTL does.

b) **Modify or skip the test.**  Document it as testing a feature h2
   doesn't provide (and arguably shouldn't, per strict POSIX — creating
   joinable threads and never joining them is user error).  Possibly
   replace with a variant that uses detached workers only, or has main
   call `exit(0)` after releasing workers instead of `pthread_exit`.

Also worth noting on inspection: our `pthread_exit` body blocks the
calling thread on `pthread_sem_wait_np(&self->joined)` unconditionally
when `!detached`.  For `main()` this means main itself blocks waiting for
a join that will never come (main is joinable by default).  That's
probably a pthread library bug independent of the test — the initial
thread should be treated specially.  Worth investigating separately.

## v68/opt pi-test HANG investigation (2026-06-29) — RACE confirmed, root-causing

### Symptom
`kernel/futex/futex/test/tests/pi` HANGS on **v68/opt only** (v68/ref PASS,
v81/opt PASS).  Test logic completes ("TEST PASSED" + "Handoff Successful")
then `exit(0)` never finishes → ulimit kill (exit 137).

### Ruled out
- **Stack smash**: doubled THREAD_STACK_SIZE 256→512, confirmed rebuild, still
  hung.  Not a stack overflow.

### Confirmed: it's a RACE (Heisenbug)
- Interactive PA dump of the hung build: `H2K_kg.ready_valids` (offset 0,
  4×u64 @ 0xff00c000) ALL ZERO → nothing runnable anywhere, incl. booter.
  Booter is asleep waiting for a child wakeup that never came.
- Adding **kernel LOGBUF logging** perturbed timing → test stopped hanging but
  FAILed "t2 not spinning" (test.c:261) — a timing-dependent assertion in the
  PI handoff.  So kernel logging is too heavy; it moves the race window.
- **Userspace-only** instrumentation (SYS_WRITECREG in sys_exit + raw __angel
  trace in pthread_exit) is light enough: test PASSES and shows a clean
  teardown.  So any logging in the kernel scheduler path hides the bug.

### Key structural findings
- Spinners call `h2_thread_stop(0)` → tailcalls `pthread_exit` →
  `pthread_safe_death` → `trap0(H2_TRAP_THREAD_STOP)` → kernel
  `H2K_thread_stop` → `thread_stop_withlock`.
- **alive_count "last thread" logic is effectively dead code for this test.**
  Only ~4 threads ever run userspace `pthread_exit`; alive_count never reaches
  0 (bottoms ~0x1a).  The 38 spinners are reaped IN-KERNEL by main's
  `exit(0)`→`h2_vmkill(SELF,0)` (skip-self), bypassing pthread_exit entirely.
- Teardown completion depends ENTIRELY on: main `exit(0)` → `h2_vmkill(SELF,0)`
  stamps vmblock->status=0, marks all others KILL+VMWORK, skips self → spinners
  reaped via `do_work_withlock`→`thread_stop_withlock(status=0)`.  With status=0
  the CHILDINT-post condition `status!=0 || num_cpus==0` is FALSE for every
  spinner reap (num_cpus bottoms at 1 because main is skipped).  CHILDINT to
  booter is posted ONLY when the genuinely-last thread brings num_cpus to 0.

### Leading hypothesis (the "am I last to exit" race)
The CHILDINT to booter (`H2K_vm_cpuint_post_locked` in stop_withlock, fired
only at num_cpus==0) either isn't delivered to booter's VM, or races with
booter's wait loop, on v68/opt specifically.

- booter waits via `h2_anysignal_wait(&wake_sig, WAKE_CHILD|WAKE_TIMER)`
  (futex-backed, LEVEL-latched — SIGNALS bit persists, so a set-before-wait
  returns immediately; intra-booter lost-wakeup is unlikely).
- booter_isr sets WAKE_CHILD on CHILDINT.  If the CHILDINT interrupt never
  reaches booter_isr (delivery race in cpuint post to a parent context that's
  mid-transition), wake_sig stays 0 and booter sleeps forever → matches
  ready_valids==0.
- v68-specific angle: switch.v4opt.S has a v68-only dmresume/dm0 block
  (lines ~360-362); the opt path also gates the vm_do_work call on
  `if (P_VMWORK)`.  Worth checking whether a KILL-marked thread can reach the
  switch with VMWORK somehow not set on v68, or whether cpuint delivery
  to booter races with the reaper.

### NEXT STEP
Trace the CHILDINT post→deliver→booter_isr path (H2K_vm_cpuint_post_locked and
vmint delivery, incl. vmint.v4opt.S) for the num_cpus==0 case.  Confirm whether
the final CHILDINT is (a) never posted, (b) posted but not delivered, or
(c) delivered but booter missed it.  Lightweight signal only — kernel logging
moves the race.  Consider: does main reliably become the last thread, and does
ITS stop post CHILDINT?

### Instrumentation currently in tree (REMOVE before commit)
- `libs/syscall/angel/src/sys_exit.c`: `dbg()` SYS_WRITECREG tracer + calls.
- `libs/posix/pthread/thread/pthread_thread.ref.c`: `pe_dbg`/`pe_dbg_hex` raw
  __angel tracer + calls; `extern __angel` decl.
- `kernel/vm/vmop/vmop.ref.c`, `kernel/thread/stop/stop.ref.c`,
  `kernel/vm/vmwork/vmwork.ref.c`: H2K_log() DBG lines + `#include <log.h>`.
- `kernel/futex/futex/test/tests/pi/test.c`: `info("TEST DONE...")` line.

### Refinement (2026-06-29 cont.): wait/wake protocol traced, BKL intact

Traced the full CHILDINT post→deliver→booter-wake protocol:
- POST: `thread_stop_withlock` (num_cpus==0 case) → `H2K_vm_cpuint_post_locked`
  (cpuint.ref.c:32).  Sets pending bit (intno=14=CHILDINT in low u16
  `cpuint_pending`; `cpuint_enabled` is high u16).  If `pending & enabled`,
  calls `H2K_vm_int_deliver_locked`.
- DELIVER: `vmint.ref.c:28`.  If booter is VMWAIT → clear waiting_cpus bit,
  set r00=intno, `H2K_ready_append`, `H2K_check_sanity`.  Wakes booter.
  Other booter states (RUNNING/READY/INTBLOCKED/BLOCKED) only act if IE set.
- WAIT: booter `H2K_vmtrap_wait` (ref vmfuncs.ref.c:107 / opt vmfuncs.v4opt.S).
  Under BKL: `do_work_withlock`→check_interrupts; if pending returns intno (no
  sleep); else set VMWAIT + waiting_cpus + dosched.

Both post and wait hold BKL (k0lock), so the basic post-vs-sleep ordering is
serialized and race-free in BOTH ref and opt.  The opt vmtrap_wait holds
k0lock across do_work + VMWAIT transition + jump to dosched — same contract
as ref.  So it's NOT a simple BKL gap.

Booter's userspace wait is `h2_anysignal_wait` (futex-backed, LEVEL-latched
SIGNALS word) — set-before-wait returns immediately, so intra-booter lost
wakeup is unlikely.  booter_isr sets WAKE_CHILD on CHILDINT guestint.

So the remaining suspects for v68/opt-only:
1. The CHILDINT cpuint is delivered to booter's kernel context (ready_append)
   but the actual guest interrupt (booter_isr) is never injected — i.e. the
   gap is between "booter VCPU readied" and "booter_isr actually runs the
   WAKE_CHILD set".  If booter was readied for some OTHER reason, ran, checked
   child status (still alive), and went back to VMWAIT, and the CHILDINT
   pending bit got consumed/cleared without injecting the guest int...
2. A v68opt-specific asm path: cpuint.v68opt.S / vmint has a v68 variant?
   Check whether vmint delivery or the guest-int injection has a v68opt
   assembly version that differs from ref.
3. Whether `H2K_check_sanity` (called from deliver) vs `check_sanity_unlock`
   interacts with the opt switch's v68 dmresume/dm0 block.

NEXT: check for vmint/guest-int-injection v68opt asm; and add a SINGLE
lightweight marker (not in scheduler hot path) at the num_cpus==0 CHILDINT
post to confirm in a HANGING run whether the post even happens.  Need a way to
log that doesn't perturb — maybe a memory breadcrumb (write a sentinel to a
known PA) rather than angel output, then dump it via the interactive PA peek.

### Finding (2026-06-29 cont.): v68/opt uses HAND-WRITTEN asm vmint/cpuint

v68/opt compiles `vmint.v68opt.S` + `cpuint.v68opt.S` (NOT the .ref.c).
v68/ref uses .ref.c.  This is the "C and ASM out of sync" surface.

DIVERGENCE FOUND (but probably NOT the v68-specific cause):
`H2K_vm_int_deliver_locked` ref C (vmint.ref.c:31-34) clears
`vmblock->waiting_cpus` bit for the woken cpu in the VMWAIT case.  The opt asm
does NOT — confirmed by grep, NONE of vmint.v{4,55,5,60,65,68,73,81}opt.S
reference waiting_cpus at all.  Since v81/opt also omits it yet PASSES, this
divergence is long-standing and consistent, so it doesn't by itself explain
v68-only hang.  (Still worth fixing/understanding separately — possible latent
bug; waiting_cpus is used to route shared ints to waiting cpus first.)

NEXT: diff vmint.v68opt.S vs vmint.v81opt.S deliver_locked (and the cpuint
v68 vs v81) to find what's actually DIFFERENT between the passing and hanging
opt variants.  Also the v68-only dmresume/dm0 block in switch.v4opt.S
(lines ~360-362) remains a prime suspect for a v68-specific timing/register
issue around the kill/reap switch.

## VMWORK contract: libs/ caller audit (2026-06-30)

Audited every guest-side caller of the block primitives in libs/ to confirm
they treat a -1 (or any unexpected) return from the block trap as a spurious
failure that must be retried.  Result: all clean -- no caller treats -1 as
success or fatal, every site has a "reload condition, jump back to trap0"
retry loop.

futex_wait callers (trap0 #H2_TRAP_FUTEX_WAIT):
- libs/posix/pthread/sem/pthread_sem.ref.S:148 -- `.Ldo_block` retry loop
- libs/posix/pthread/barrier/pthread_barrier.ref.S:109 -- `.Lblock_again`
- libs/posix/pthread/cond/pthread_cond.ref.S:157 -- label `2:` retry
- libs/posix/pthread/rwlock/pthread_rwlock.ref.S:118 -- `.Lrd_wait` (read)
- libs/posix/pthread/rwlock/pthread_rwlock.ref.S:176 -- `.Lwr_wait` (write)
- libs/h2_compat/signal/h2_signal.ref.S:43 -- jumps back to h2_signal_wait_any
- libs/h2_compat/signal/h2_signal.ref.S:79 -- jumps back to h2_signal_wait_all
- libs/h2_compat/anysignal/h2_anysignal.ref.S:42 -- explicit "spurrious wakeup"
  comment in code (existing)
- libs/h2_compat/allsignal/h2_allsignal.ref.S:70 -- "should not happen" comment
  but the retry is present

futex_lock_pi callers (trap0 #H2_TRAP_FUTEX_LOCK_PI):
- libs/posix/pthread/plainmutex/pthread_plainmutex.ref.S:37 -- leaf primitive
  has `_canfail` suffix and returns kernel r0 directly.  Its C wrapper at
  libs/posix/pthread/plainmutex/pthread_plainmutex_imp.ref.c:15 wraps in
  `while (lock_id_canfail_np(...) != 0)` -- proper retry.

popup_wait callers: ZERO call sites in libs/ -- popup primitive defined but
not consumed.

intpool_wait callers: ZERO call sites in libs/ -- same.  Both are exposed
through libs/h2/intpool/ but not wired up to any higher-level API today.

Conclusion: the VMWORK gate fix can return -1 spuriously from every block
primitive without breaking any existing guest caller.  popup/intpool callers
are nonexistent in-tree, so their fix is purely defensive (per the
"all block points honor VMWORK" contract in vmdefs.spec).

Minor observation: pthread_cond_imp.ref.c:25 already has a comment noting
that futex can fail spuriously ("we can request a new timer event and that
will cause the futex to fail").  The retry mindset already exists in the
codebase; we are just formalizing the kernel-side guarantee.

## VMWORK contract: futex unit test pre-fix failure (2026-06-30)

Confirmed kernel/futex/futex/test/tests/vmwork/test.c fails pre-fix with the
expected gate-doesn't-fire signature.  Test extended to add two new cases:
VMWORK alone (case B, assertion flipped from old broken behavior) and
VMWORK|KILL (case C, the actual kill-thread scenario from VMKILL_race.md).

Pre-fix run (v81/opt default, archsim standalone):

	FAIL: B: VMWORK alone must gate (wait)
	FAIL: B: VMWORK alone must gate (lock_pi)
	FAIL: C: VMWORK|KILL must gate (wait)
	FAIL: C: VMWORK|KILL must gate (lock_pi)
	FAIL
	exit status == 0    [simulator success; test program exited via exit(1)]
	make: *** [...:495: results.txt] Error 1

Test was refactored from halt-on-first-FAIL to non-fatal CHECK that records
failures and continues, so we see all four failing sub-assertions in a single
run.  Case A (VMWORK|IE) passes both pre- and post-fix because the existing
gate `VMWORK && IE` already handles that path correctly -- there is no
behavior regression to test for there.

Failure root cause: the current gate at
kernel/futex/futex_classic/futex_classic.ref.c:27 is
`(VMWORK & vmstatus) && (IE & vmstatus)` -- requires BOTH bits.  With only
VMWORK set (case B) or VMWORK|KILL (case C, no IE) the gate falls through
to safemem, vm_do_work is never called, TH_saw_do_work stays 0.

Post-fix (gate on VMWORK alone) the test should print only "TEST PASSED".

### Post-fix confirmation (2026-06-30)

After applying the gate fix to kernel/futex/futex_classic/futex_classic.ref.c
and kernel/futex/futex_pi/futex_pi.ref.c -- VMWORK-only gate, and calling
H2K_vm_do_work_withlock directly (no unlock-then-relock around the wrapper) --
the test prints "TEST PASSED".

Two non-obvious things needed to actually exercise the change:

- The standalone test links against `artifacts/v$(ARCHV)/$(TARGET)/install/lib/libh2kernel.a`.
  TARGET defaults to opt (Makefile.inc.config:55), which builds futex_classic
  from `.v4opt.S` -- the .ref.c is not in the opt library.  To exercise the
  ref C path, build with `TARGET=ref make` from the project root and run
  the test from its directory.

- The test stubs `H2K_vm_do_work`.  Switching futex_classic to call
  `H2K_vm_do_work_withlock` (correct: BKL is already held) requires the
  test to stub that name instead; otherwise the real `withlock` impl is
  linked in and dereferences `me->vmblock->status` on a zero-initialized
  stub context -> hang/crash.

### Opt-path fix confirmed (2026-06-30)

After applying the same changes to kernel/futex/futex_classic/futex_classic.v4opt.S
and kernel/futex/futex_pi/futex_pi.v4opt.S (mask = VMWORK alone; call
H2K_vm_do_work_withlock with BKL held, then k0unlock, then return -1),
the vmwork unit test prints "TEST PASSED" against both targets:

  TARGET=ref make   # ref C path
  make              # default TARGET=opt; exercises v4opt.S asm

Both paths now exercise the new VMWORK-alone gate.  The fix for the
v68/opt pi-test hang requires both halves -- the asm change is the one
that actually closes the production race; the ref change keeps the C
reference in sync.

## vmkill_vm userspace-deadlock fix (2026-06-30)

Second root cause found after the VMWORK gate fix: after VMOP_KILL_VM_SELF,
some threads die (via IPI/kill) without going through pthread_exit, so they
never decrement alive_count.  When main's pthread_exit runs, last==false,
and it blocks waiting for threads that are already dead.  VM never drains.

Final fix: removed "if (target == me) continue" from H2K_vmop_kill_vm kill
loop.  Caller now gets KILL|VMWORK + IPI like all other RUNNING threads.
IPI fires on first userspace instruction after vmkill returns; caller cannot
reach any mutex before being killed.

pthread_exit defense-in-depth (libs/posix/pthread/thread/pthread_thread.ref.c):
in last==true branch, after h2_vmkill, call h2_thread_stop_trap(0).  Now
redundant (IPI fires first) but kept as belt-and-suspenders.

## Session summary (2026-06-30): 1224 passed, 0 failed across 9 variants

Two root causes closed for v68/opt pi-test hang (TEST PASSED but never drained):

1. VMWORK gate fix: futex_classic + futex_pi (ref .c and .v4opt.S).
   Gate changed from VMWORK&&IE to VMWORK alone.  Directed unit test added.

2. vmop_kill_vm caller-kill: removed caller-skip from kill loop.  Caller gets
   IPI with siblings; IPI fires before any blocking call can be made.

Specs updated: vmdefs.spec, vmwork.spec, futex_classic/pi/popup/intpool specs,
vmop.spec.

Open for next session: #4 vmwork unconditional-clear test; #7 PI owner/waiter
audit; #11 retry backstop; vmwork.ref.c unconditional clear (spec already done).

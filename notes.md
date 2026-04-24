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
- Final artifacts in `artifacts/v$(ARCHV)/$(T)/install/`: `bin/booter`, `lib/libh2.a`, etc.
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
   FIXED (2026-04-23): Test artifacts now land in `artifacts/v$(ARCHV)/$(T)/build/tests/<test-path>/`.
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
booter/qurt/angel .o files) are now under `artifacts/v$(ARCHV)/$(T)/build/`.

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

**NEXT TASK:** `make testall` — validate and enable parallel cross-ARCHV builds.
All output files are now in per-variant artifact dirs and `$(ARCHV_LIST): testall_prepare`
ordering is in place.  Steps:
1. Run `make testall` (sequential) to confirm all ARCHV × opt/ref variants pass.
2. Run `make -j4 testall` to validate true parallel execution.
3. Investigate any failures; fix any remaining conflicts that surface.

Remaining in-source issue (FIXME in Makefile.qurt): per-test qurt support libraries
(`$(QURT_TEST_LIBS)::`) still build in-source via `cd $@`. Needs the same out-of-tree
treatment as the per-test $(SUBDIRS) rule. Low priority — qurt test source doesn't
exist in this repo yet.

Dead code in Makefile.inc.test: the old `report.html`, `h2_report.html`,
`qurt_report.html` targets (lines ~238-266) are no longer reachable from any build
path — both h2 and qurt now use the $(INSTALLPATH)/... targets directly. Can be
removed in a future cleanup pass.

- `test.out`: fresh on each `make test` run. SUMMARY line first, then per-test PASS/FAIL.
- `make.log`: raw parallel test run output (overwritten each run). Check for build failures.
- `artifacts/v$(ARCHV)/$(T)/build/tests/<test-path>/make.log`: full output for one test. Look here when a specific test fails.
- `h2_report.html`: HTML version of per-test PASS/FAIL summary.
- `testall` (multi-arch CI): uses `NO_TEST_RESET=1` to accumulate results across arch runs.

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

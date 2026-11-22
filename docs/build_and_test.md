
# Building and Testing

Variants are selected with the `TARGET` variable, not with a target name:

```
make ARCHV=81 TARGET=opt BATCH=0 build          # build one variant locally
make ARCHV=81 TARGET=opt BATCH=0 test_variant   # build + test it; see artifacts/v81/opt/install/test.out
make BATCH=0 testall                            # build + test every ARCHV x variant
```

The top-level `makefile` (no `Makefile`) is a wrapper that forwards every goal
to `make -f makefile.real`.  Always name the goal: a bare `make` runs the
wrapper's first target and builds nothing.

`BATCH=1` (default) submits through `bsub -I` with `TEST_JFLAG=-j8`.  Use
`BATCH=0` where bsub is not available: the build runs locally with
`-j$(nproc)`.  Either way an inherited or user-supplied `-jN` wins, so pass
`make -j8` (or any number) to limit parallelism on a shared machine.
With `BATCH=1` each test is a job on an LSF host, and this host's NFS cache can
be up to 60 s stale when bsub returns.  So each job does its own bookkeeping
(the `job` target in `scripts/Makefile.inc.test`), and `h2_test` waits
`NFS_SETTLE` seconds (default 75, 0 disables) before reading the results.
The default variant is `ARCHV=81 TARGET=opt` (`scripts/Makefile.inc.config`).
`make test_variant` runs the incremental build itself, so you do **not** need a
separate build step: edit a kernel, libs, booter or test source and re-run it.
`make test` is a synonym for `make testall`.

`TARGET` selects the variant.  `TARGET=opt` builds with optimizations enabled.
`TARGET=ref` uses the reference versions of implementations rather than the
optimized ones.  `TARGET=opt_si` is opt with the image linked at 0x87000000.
We want to test both opt and ref implementations to ensure they are kept in sync.
`make testall` covers every combination in `ARCHV_LIST` (68 73 81) x `VARIANTS`
(opt ref opt_si); see `scripts/Makefile.inc.config`.

Outside BATCH, `IGNOREFAIL=1` is the default: a failing variant does not stop
`make testall` from building the unified report.

## Incremental runs

Re-running is dependency-driven, so repeat invocations do only the work that is
actually needed:

* No source change: a true no-op.  No test dir is entered (~18 s for testall).
* A change to the kernel, libs or booter bytes: every test that links them reruns
  (`libh2kernel.a` is linked into every test binary).  A source edit that leaves
  the archive bytes the same (a comment, an unused `-D`) recompiles but retests
  nothing.
* A single test's own sources change: only that test reruns, in every variant
  that has it.

How:

* Objects track their headers through depfiles (`DEPFLAGS := -MMD -MP`, the `.d`
  files included by each Makefile); the staged headers are order-only.
* Component archives are always re-checked by their sub-make (FORCE in
  `libs/Makefile`).
* Changing a compile flag or define (command line or
  `scripts/Makefile.inc.config`) rebuilds exactly what uses it: every build
  directory keeps a `flags.stamp` (`scripts/Makefile.inc.flagstamp`), and the
  consts.pl values have a `consts.stamp`.  The git commit is a kernel define,
  so each commit rebuilds the kernel and reruns all tests.
* Every installed file is a real make target whose prerequisite is the built
  file it comes from (`install_files` in `scripts/Makefile.inc.install`),
  copied with `cp -p`.
* Tests rerun on `install/manifest` (below), not on install mtimes: the test
  driver always runs the incremental build first, and the manifest only changes
  when the archive or booter bytes do.  `stake/syms.pl` sorts its symbol output
  so `booter` is byte-reproducible.
* Each variant's `install/test_results.json` is FORCE'd: its rule runs
  `test_variant`, whose sub-make reruns only the out-of-date `results.txt`
  files and rewrites the json only when one changed.

## Running a single test directly

```
cd kernel/data/intconfig/test && make TARGET=opt ARCHV=81
```

This bootstraps through the top-level build first, then re-enters as a sub-make,
so a kernel edit is picked up by that same invocation.  Explicit low-level goals
(`make test.log`, `make test.elf`) skip the bootstrap and only check that the
install tree exists; build from the top level first if you use those.

Fast path: ask the test driver for one `results.txt` (it builds the variant's
install first):

```
H2DIR=$(pwd) INSTALLPATH=$(pwd)/artifacts/v81/opt/install \
  make -f scripts/Makefile.coverage ARCHV=81 TARGET=opt ULIMIT=30 \
    $(pwd)/artifacts/v81/opt/build/tests/<test-path>/results.txt
```

* Pass = `results.txt` contains `TEST PASSED` and the test's `make.log` has
  `exit status == 0` (137 = killed by ULIMIT).
* `results.txt` is up to date until an input changes; delete it to force a rerun.
* `ULIMIT` is CPU seconds for the simulator (`ulimit -t`, default 600 in
  `scripts/Makefile.inc.test`).  Too small looks like a hang.
* Run with `BATCH=0` (or, as here, outside makefile.real, where `BSUB` is
  empty) on hosts without bsub; plain `make test` uses `BATCH=1`.
* The simulator runs without `--profile` (it doubles simulator time); only the coverage
  rules (and `Makefile.qurt` `all`) set `PROFILE=--profile`, since only they read the gmon
  files.  `test.log` deletes old gmon files first, so a run never leaves stale ones behind.
* Running the simulator by hand outside make: set `PKW_VERSIONS` as in
  `scripts/Makefile.inc.tools`, else the V60 archsim on `PATH` hangs silently.
* Tests are registered in `scripts/testlist.v61`; `testlist.v62` .. `v81` are
  symlinks to it.  Edit v61 only; `sed -i` on a link replaces it with a copy.

By setting the ARCHV variable on the make command line, like `ARCHV=68` we can
change the architecture that we build for.  When the project started there were
substantial changes between architectures that affected monitor mode
and how code was optimized.

Some architectures are similar enough that we can reuse builds for multiple versions.
`make testall` covers `ARCHV_LIST` = 68 73 81.  Older ARCHV values (e.g. 65)
are still configured, but v65 opt is not known to build.

# Output Files

`test.out` — written fresh on each `make test` run.  Contains a one-line
`SUMMARY: N passed, M failed` followed by per-test PASS/FAIL detail from
`check-fail`.  This is the first file to look at after a test run.

`artifacts/v$(ARCHV)/$(TARGET)/install/make.log` — raw interleaved output from the
parallel test run (build + simulator output for all tests combined).  Useful for
diagnosing build failures that prevent tests from compiling.

`artifacts/v$(ARCHV)/$(TARGET)/build/tests/<test-path>/make.log` — full output for
one specific test.  Look here when a specific test fails.

`build.log` — not generated automatically; capture it with
`make BATCH=0 TARGET=opt build 2>&1 | tee build.log` when you need to review build warnings.

`artifacts/v$(ARCHV)/$(TARGET)/install/test_report.html` — HTML summary of PASS/FAIL
per test, generated by `make test`.

`artifacts/test_report.html` — unified HTML report across all ARCHV×variant
combinations, generated by `make testall`.

`artifacts/test.out` — global one-line `SUMMARY: N passed, M failed across K variant(s)`
written by `make testall`.

`artifacts/size_report.md` — size report for the opt variants, written by
`make sizeall` (not by `testall`).  It compares `libh2kernel.a` (with the objects
that changed), `libh2.a` and `bin/booter` against a baseline install for each
ARCHV: `SIZE_BASELINE=<dir>` (`{archv}` is replaced, e.g. `/path/h2/v{archv}_latest`),
or with `USE_PKW=1` the pkw h2 release (`PKW_h2=v<ARCHV>_latest pkw --env h2`).
No baseline, or one that is not found, is an error.  `make size` builds one
variant and writes `install/size_report.md`.

# Artifacts

Build output goes under `artifacts/` (or `artifacts_picolibc/` with `PICOLIBC=1`):
objects are under `build/`, and `install/` holds the files collected after they are
built.

In the `install/bin` directory is `booter`, our binary that loads up other programs 
into their own VM to execute.  This is the primary workhorse for running tests.

In the `install/lib` directory there is `libh2.a` which contains helper functions for
user/guest programs, as well as `libh2kernel.a` which contains most of the code 
and data for monitor mode execution.

Unsuprisingly, `install/include` contains header files.  Each one has its own
install rule (`scripts/Makefile.inc.headers`, included by `libs/Makefile` and
`scripts/Makefile.coverage`), and every test's `results.txt` depends on the whole
installed set: `make test_variant` (or a single `results.txt`) installs a new or
edited guest header first, then the test's own `.d` file recompiles only the tests
that include it.

`install/manifest` holds sha256sums of `libh2kernel.a`, `libh2.a`,
`libh2check.a` and `booter`, rewritten only when they change.  The test driver
(`scripts/Makefile.coverage`) always runs the incremental `build` first, and
tests depend on the manifest, so they never run against stale libraries and a
no-op build reruns nothing.

`install/src` contains some files that are often needed for building programs,
such as `min_crt0.S`, `bootvm.S`, and `bootvm_entry.S`.

`install/scripts` contains configuration files and scripts, including linker
scripts (`vapa_bootvm.x`, `vapa_kernel.x.in`) and simulator device configs
(`devsim_v*.cfg`).

`install/stake` contains build support files used to compute kernel sizing.

# LLDB Debugging

Generate an LLDB setup file configured with your local build paths:

```
make lldb-setup                  # generate lldb-setup (default port 1234)
make lldb-setup LLDB_PORT=6789   # override the gdb-remote port
make lldb-setup-clean            # remove generated file
```

The generated file is gitignored; each developer generates their own.

# Picolibc

`PICOLIBC=1` builds against Picolibc instead of Dinkumware, into `artifacts_picolibc/`
(a sibling of `artifacts/`).  `make all_clean` removes only the tree that `PICOLIBC`
selects.

    /usr/bin/make -O BATCH=0 USE_PKW=0 NULL_ANGEL_TRAP=1 PICOLIBC=1 TOOLCHAIN_BIN=<toolchain>/bin testall

`TOOLCHAIN_BIN` is put first on `PATH`.  The directory above it must contain
`templates/` and `target/picolibc/`.  Use GNU make 4.2 or newer (`/usr/bin/make` is
4.3; a plain `make` may be 3.81).  `BATCH=0` runs locally, and `USE_PKW=0` runs the
tests on the toolchain's simulator instead of the pkw devsim.  The local commands
pass `NULL_ANGEL_TRAP=1` for every variant; the Makefile sets it itself only for
`opt_si`.  CI runs with `USE_PKW=1` (archsim, the pkw devsim) and `BATCH=1`, so each
build and test is a `bsub -I` job.  It passes `MAGIC_ANGEL=--magic_angel` for every
variant and `IGNOREFAIL=1`, so the report is always written and the check step is the
gate.  With `BATCH=0`, `IGNOREFAIL` defaults to 1, so read `artifacts_picolibc/test.out`,
not the exit status.

A passing run reports `SUMMARY: 1224 passed, 0 failed across 9 variant(s)`: 136 tests
for each of ARCHV 68, 73, 81 and each of the variants opt, ref, opt_si.

CI takes the toolchain from pkw (`hexagon-tools=23.0`) and copies it without
`target/hexagon`, `target/hexagon-unknown-qurt-dw` and `target/qnx`, so a build that
misses `--cstdlib=picolibc` fails.  The copy lives in `RUNNER_TEMP`, under the runner's
`_work` directory with the checkout, rather than in `/tmp`, which is local to the runner host.

# WIP: Log Spam Control (Phase 3)

## Branch: wip/log-spam-control

## What's done

### log.h
- `H2K_log_once(fmt, ...)` — fires once globally per callsite (callsite-static `u32_t __f`)
- `H2K_log_once_ht(fmt, ...)` — fires once per hardware thread (callsite-static `u32_t __f[MAX_HTHREADS]`)
- `H2K_log_throttle(interval, fmt, ...)` — rate-limit globally (callsite-static `u64_t __t`)
- `H2K_log_throttle_ht(interval, fmt, ...)` — rate-limit per hardware thread
- Severity variants for all four: `_err`, `_warn`, `_info` suffixes
  - e.g. `H2K_log_once_err(...)`, `H2K_log_throttle_ht_warn(interval, ...)`
- All three build branches consistent: `H2K_LOGBUF` (full), `H2K_LOG_PRINTF` (pass-through), off (no-op)

### log.ref.c
- `H2K_log_once(u32_t *flag)` — `H2K_atomic_setbit`, returns 1 on first call
- `H2K_log_once_ht(u32_t *flags)` — per-ht slot, lockless
- `H2K_log_throttle(u64_t *last, u64_t interval)` — pcycle delta, uses `logbuf_lock`
- `H2K_log_throttle_ht(u64_t *last, u64_t interval)` — per-ht slot, lockless

### log_diagram.excalidraw
- Updated compile-time flags box: removed `H2K_LOG_PRINTF`, added `make LOGBUF=sched` examples

## Known issues / TODO

1. **`H2K_LOG_PRINTF` inconsistency** — removed from diagram but still in `log.h` code.
   Decision needed: remove from code too, or restore in diagram?

2. **Global throttle deadlock risk** — `H2K_log_throttle` takes `logbuf_lock`.
   Safe today (never called while lock is held), but fragile. Document or enforce.

3. **`MAX_HTHREADS` hidden dependency** — `_ht` macros require `max.h` included
   before `log.h`. Works in kernel, but not obvious from the header alone.

4. **Inline header problem (deferred)** — callsite-static `__f` is duplicated
   per translation unit for `static inline` functions in headers (e.g. `readylist.h`).
   Workaround: channel-ID design using `H2K_gp` state (see plan file). Deferred.

## How spam control works

Each macro expansion creates its own `static` local variable at that point in source:
- `once`: `static u32_t __f = 0` — atomically set on first call, suppressed after
- `throttle`: `static u64_t __t = 0` — timestamp of last fire, checked against interval

Per-function independence: each expansion site has its own `__f`/`__t`, so two
`H2K_log_once()` calls in different functions fire independently (once each).

Time unit for throttle: CPU pcycles via `H2K_get_pcycle_reg()` — same concept as
Linux `printk_ratelimited` but cycle-based instead of wall-clock milliseconds.

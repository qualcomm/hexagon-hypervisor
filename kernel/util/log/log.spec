/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

# H2K Logging Specification

## Build Configuration

### LOGBUF Syntax

```
make LOGBUF=<target>[<level>]
```

- `<target>`: `1` (all), `<module>` (dir), `<file>` (filename), or comma-separated list
- `<level>`: `0`/`err`, `1`/`warn`, `2`/`info`, `3`/`dbg` (default: `3`)

Examples:
```
make LOGBUF=1[warn]              # whole kernel, WARN+
make LOGBUF=sched[info]          # sched module, INFO+
make LOGBUF=sched[warn],futex[err]  # mixed levels
```

### H2K_LOG_NO_SPAM

Disables spam-control macros (once/throttle) to eliminate static state:
```
make LOGBUF=1[warn] H2K_LOG_NO_SPAM=1
```

When enabled, `H2K_log_once*()` and `H2K_log_throttle*()` become no-ops.

## API

### Base Logging
```c
H2K_log(fmt, ...)        // DEBUG
H2K_log_err(fmt, ...)    // ERROR
H2K_log_warn(fmt, ...)   // WARN
H2K_log_info(fmt, ...)   // INFO
H2K_log_dbg(fmt, ...)    // DEBUG
```

### Spam Control
```c
H2K_log_once*(fmt, ...)           // emit once per callsite (4 bytes)
H2K_log_once_ht*(fmt, ...)        // emit once per HT (4 bytes, bitmask)
H2K_log_throttle*(interval, ...)  // rate-limited (8 bytes)
H2K_log_throttle_ht*(interval, ...)  // per-HT rate-limited (8×MAX_HTHREADS bytes)
```

All combinations of severity (`_err`, `_warn`, `_info`, `_dbg`) available.

### String Variants
```c
H2K_log_string*(S)              // pre-formatted string
H2K_log_string_once*(S)         // with spam control
H2K_log_string_throttle*(interval, S)
```

## Output Format

```
[ht<id>][<LEVEL>][<pcycles>][<file>:<line>] message
```

Example: `[ht0][WARN][12345678][sched.c:42] reschedule timeout`

## Memory

| Macro | Storage |
|-------|---------|
| `H2K_log_once()` | 4 bytes (u32 flag) |
| `H2K_log_once_ht()` | 4 bytes (u32 bitmask, 1 bit per HT) |
| `H2K_log_throttle()` | 8 bytes (u64 timestamp) |
| `H2K_log_throttle_ht()` | 8 × MAX_HTHREADS bytes |

With `H2K_LOG_NO_SPAM=1`: no static state for spam-control macros.

## Implementation

- **H2K_LOGBUF**: lockless per-HT ring buffers, atomic setbit for once, spinlock for throttle
- **H2K_LOG_PRINTF**: printf-based, static inline prefix helpers, same spam-control logic
- **Silent**: all macros are no-ops (default when neither flag set)

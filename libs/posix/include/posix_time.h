/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_POSIX_TIME_H
#define H2_POSIX_TIME_H 1

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h> // dinkumware time.h, if we change c libraries we probably need to nuke this
#include <h2.h>

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

/* EJP: for now, lie about nanosleep.
 * We can actually nanosleep, see h2_nanosleep, but it might not be conformant and we have to simulate with timer.
 */
static inline int nanosleep(const struct timespec *req, struct timespec *rem) { return 0; }

typedef enum {
	CLOCK_MONOTONIC,
	CLOCK_THREAD_CPUTIME_ID,
	CLOCK_INVALID
} clockid_t;

#define CLOCK_REALTIME CLOCK_MONOTONIC
#define CLOCK_MONOTINC_RAW CLOCK_MONOTONIC
#define CLOCK_PROCESS_CPUTIME_ID CLOCK_THREAD_CPUTIME_ID
#define _POSIX_MONOTONIC_CLOCK 0
#define _POSIX_CPUTIME 0
#define _POSIX_THREAD_CPUTIME 0
#define _POSIX_TIMERS 1

int clock_gettime(clockid_t clock_id, struct timespec *tp);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

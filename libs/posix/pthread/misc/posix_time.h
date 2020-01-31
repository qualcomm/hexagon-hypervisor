/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_POSIX_TIME_H
#define H2_POSIX_TIME_H 1

#define _PROVIDE_POSIX_TIME_DECLS 1
#include <time.h> // dinkumware time.h, if we change c libraries we probably need to nuke this

/* In case time.h was included early without defining
	 _PROVIDE_POSIX_TIME_DECLS. This can happen as a side-effect when
	 applications include other system includes before including h2.h, pthread.h,
	 or this file */

// defined in time.h when _PROVIDE_POSIX_TIME_DECLS != 0
#ifndef CLOCK_REALTIME

#ifdef __cplusplus
extern "C" {
#endif

struct timespec {
       time_t tv_sec;
       long tv_nsec;
};

/* nanosleep implementation will be provided by the OS library
 */
int nanosleep(const struct timespec *req, struct timespec *rem);

#define CLOCK_REALTIME (0)
#define CLOCK_MONOTONIC (1)
#define CLOCK_THREAD_CPUTIME_ID (2)
#define CLOCK_PROCESS_CPUTIME_ID (3)

/* clock_gettime implementation will be provided by the OS library
 */
int clock_gettime(clockid_t clock_id, struct timespec *tp);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

#endif

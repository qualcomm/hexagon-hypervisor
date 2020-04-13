/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _H2_POSIX_SCHED_H
#define _H2_POSIX_SCHED_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <h2if.h>

struct sched_param { 
	int sched_priority;
};

static inline int sched_yield(void) { h2_yield(); return 0; }

/* PID-based schedparam stuff may need to go here */

/* sched priority stuff may need to go here */

#ifdef __cplusplus
} // extern "C" 
#endif

#endif

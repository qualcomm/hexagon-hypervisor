/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TIMEINFO_H
#define H2K_TIMEINFO_H 1

#include <c_std.h>
#include <spinlock.h>
#include <tree.h>

typedef u64_t nsec_t;
typedef u64_t ticks_t;

typedef struct {
	ticks_t next_ticks;		/* next hw interrupt "ticks" */
	ticks_t last_ticks;		/* Tick of last HW interrupt */
	u64_t last_pcycles;		/* pcycle time last updated */
	H2K_treenode_t *timeouts;	/* all timeouts */
	u32_t volatile *devptr;
} __attribute__((aligned(32))) H2K_timeinfo_t;

#define H2K_TIME_FOREVER (~0ULL)
#define H2K_TIME_BIGBANG 0ULL

#define H2K_TIME_GUESTINT 12

#endif


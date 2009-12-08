/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_BARRIER_H
#define H2_BARRIER_H 1

#define H2_BARRIER_SERIAL_THREAD 1
#define H2_BARRIER_OTHER 0

#ifndef ASM
#include <h2_mutex.h>

typedef union {
	struct {
		unsigned short count;
		unsigned short threads_left;
		unsigned int threads_total;
	};
	unsigned long long int raw;
} h2_barrier_t;

static inline int h2_barrier_init(h2_barrier_t *barrier, unsigned int threads_total)
{
	barrier->count = 0;
	barrier->threads_left = barrier->threads_total = threads_total;
	return 0;
}

int h2_barrier_wait(h2_barrier_t *barrier);
#endif

#endif


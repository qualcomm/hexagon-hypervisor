/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_BARRIER_H
#define BLAST_BARRIER_H 1

#define BLAST_BARRIER_SERIAL_THREAD 1
#define BLAST_BARRIER_OTHER 0

#ifndef ASM
#include <blast_mutex.h>

typedef union {
	struct {
		unsigned short count;
		unsigned short threads_left;
		unsigned int threads_total;
	};
	unsigned long long int raw;
} blast_barrier_t;

static inline int blast_barrier_init(blast_barrier_t *barrier, unsigned int threads_total)
{
	barrier->count = 0;
	barrier->threads_left = barrier->threads_total = threads_total;
	return 0;
}

int blast_barrier_wait(blast_barrier_t *barrier);
#endif

#endif


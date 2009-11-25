/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_COND_H
#define BLAST_COND_H 1

#include <blast_mutex.h>

typedef union {
	unsigned int raw;
	struct {
		unsigned short count;
		unsigned short n_waiting;
	};
} blast_cond_t;

void blast_cond_signal(blast_cond_t *cond);
void blast_cond_broadcast(blast_cond_t *cond);
void blast_cond_wait(blast_cond_t *cond, blast_mutex_t *mutex);
static inline void blast_cond_init(blast_cond_t *cond) { cond->raw = 0; };

#endif


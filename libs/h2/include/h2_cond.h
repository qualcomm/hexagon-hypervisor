/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COND_H
#define H2_COND_H 1

#include <h2_mutex.h>

typedef union {
	unsigned int raw;
	struct {
		unsigned short count;
		unsigned short n_waiting;
	};
} h2_cond_t;

void h2_cond_signal(h2_cond_t *cond);
void h2_cond_broadcast(h2_cond_t *cond);
void h2_cond_wait(h2_cond_t *cond, h2_mutex_t *mutex);
static inline void h2_cond_init(h2_cond_t *cond) { cond->raw = 0; };

#endif


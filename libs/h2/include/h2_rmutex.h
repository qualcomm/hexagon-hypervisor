/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_RMUTEX_H
#define H2_RMUTEX_H 1

#include <h2_mutex.h>

typedef struct {
	h2_mutex_t mutex;
	unsigned int depth;
	unsigned int owner_id;
} h2_rmutex_t;

#define H2_RMUTEX_T_INIT { H2_MUTEX_T_INIT, 0, 0 }

void h2_rmutex_lock(h2_rmutex_t *lock);
void h2_rmutex_unlock(h2_rmutex_t *lock);
int h2_rmutex_trylock(h2_rmutex_t *lock);
void h2_rmutex_init(h2_rmutex_t *lock);

#endif

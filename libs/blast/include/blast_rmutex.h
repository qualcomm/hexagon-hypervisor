/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_RMUTEX_H
#define BLAST_RMUTEX_H 1

typedef struct {
	blast_mutex_t mutex;
	unsigned int depth;
	unsigned int owner_id;
} blast_rmutex_t;

void blast_rmutex_lock(blast_rmutex_t *lock);
void blast_rmutex_unlock(blast_rmutex_t *lock);
int blast_rmutex_trylock(blast_rmutex_t *lock);
void blast_rmutex_init(blast_rmutex_t *lock);

#endif

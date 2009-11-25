/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * BLAST mutex.h
 * 
 * This is a mostly-userspace mutex, but will call the 
 * kernel to block if the mutex is taken 
 */

#ifndef BLAST_MUTEX_H
#define BLAST_MUTEX_H 1

typedef unsigned int blast_mutex_t;

void blast_mutex_lock(blast_mutex_t *lock);		/* blocking */
void blast_mutex_unlock(blast_mutex_t *lock);	/* unlock */
int blast_mutex_trylock(blast_mutex_t *lock);	/* just try... 1 if successful, 0 if not */
static inline void blast_mutex_init(blast_mutex_t *lock) { *lock = 0; }	/* initialize it... */

#endif


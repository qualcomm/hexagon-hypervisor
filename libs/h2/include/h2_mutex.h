/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * H2 mutex.h
 * 
 * This is a mostly-userspace mutex, but will call the 
 * kernel to block if the mutex is taken 
 */

#ifndef H2_MUTEX_H
#define H2_MUTEX_H 1

typedef unsigned int h2_mutex_t;

#define H2_MUTEX_T_INIT 0 

void h2_mutex_lock(h2_mutex_t *lock);		/* blocking */
void h2_mutex_unlock(h2_mutex_t *lock);	/* unlock */
int h2_mutex_trylock(h2_mutex_t *lock);	/* just try... 1 if successful, 0 if not */
static inline void h2_mutex_init(h2_mutex_t *lock) { *lock = h2_mutex_t_init; }	/* initialize it... */

#endif


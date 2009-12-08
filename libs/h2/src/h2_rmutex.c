/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_rmutex.c
 * 
 * This file contains the implementation for a recursive mutex
 */

#include "h2.h"

void h2_rmutex_init(h2_rmutex_t *lock)
{
	h2_mutex_init(&lock->mutex);
	lock->depth = 0;
	lock->owner_id = 0;
}

void h2_rmutex_lock(h2_rmutex_t *lock)
{
	unsigned int my_id = h2_thread_myid();
	while (1) {
		if (h2_mutex_trylock(&lock->mutex) == 0) {
			/* Trylock succeeded, set depth and owner */
			lock->depth = 1;
			lock->owner_id = my_id;
			break;
		} else if (lock->owner_id == my_id) {
			/* Trylock failed, but owner is me, so increment depth */
			lock->depth++;
			break;
		} else {
			/* Block until mutex is freed */
			h2_mutex_lock(&lock->mutex);
			/* Lock was freed, set depth and owner */
			lock->depth = 1;
			lock->owner_id = my_id;
		}
	}
}

void h2_rmutex_unlock(h2_rmutex_t *lock)
{
	/* Decrement Depth */
	lock->depth--;
	/* If lock no longer held, unlock mutex */
	if (lock->depth == 0) {
		h2_mutex_unlock(&lock->mutex);
	}
}

int h2_rmutex_trylock(h2_rmutex_t *lock)
{
	unsigned int my_id = h2_thread_myid();
	if (h2_mutex_trylock(&lock->mutex) != 0) {
		/* Trylock succeded, set depth and owner */ 
		lock->depth = 1;
		lock->owner_id = my_id;
		return 0;
	} else if (lock->owner_id == my_id) {
		/* Trylock failed, but owner is me, so increment depth */
		lock->depth++;
		return 0;
	} else {
		/* trylock failed, I am not owner, return failure */
		return -1;
	}
}


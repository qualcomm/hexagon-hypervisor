/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_rmutex.c
 * 
 * This file contains the implementation for a recursive mutex
 */

#include "blast.h"

void blast_rmutex_init(blast_rmutex_t *lock)
{
	blast_mutex_init(&lock->mutex);
	lock->depth = 0;
	lock->owner_id = 0;
}

void blast_rmutex_lock(blast_rmutex_t *lock)
{
	unsigned int my_id = blast_thread_myid();
	while (1) {
		if (blast_mutex_trylock(&lock->mutex) == 0) {
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
			blast_mutex_lock(&lock->mutex);
			/* Lock was freed, set depth and owner */
			lock->depth = 1;
			lock->owner_id = my_id;
		}
	}
}

void blast_rmutex_unlock(blast_rmutex_t *lock)
{
	/* Decrement Depth */
	lock->depth--;
	/* If lock no longer held, unlock mutex */
	if (lock->depth == 0) {
		blast_mutex_unlock(&lock->mutex);
	}
}

int blast_rmutex_trylock(blast_rmutex_t *lock)
{
	unsigned int my_id = blast_thread_myid();
	if (blast_mutex_trylock(&lock->mutex) != 0) {
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


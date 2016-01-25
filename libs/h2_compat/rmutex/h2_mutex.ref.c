/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_mutex.c
 * 
 * This file contains the implementation for a recursive mutex
 */

#include "h2.h"

void h2_mutex_lock(h2_mutex_t *lock)
{
	unsigned int my_id = h2_thread_myid();
	if (h2_plainmutex_trylock(&lock->mutex) == 0) {
		/* Trylock succeeded, set depth and owner */
		lock->depth = 1;
		lock->owner_id = my_id;
	} else if ((lock->type != H2_MUTEX_PLAIN) && (lock->owner_id == my_id)) {
		/* Trylock failed, but owner is me, so increment depth */
		lock->depth++;
	} else {
		/* Block until mutex is freed */
		h2_plainmutex_lock(&lock->mutex);
		/* Lock was freed, set depth and owner */
		lock->depth = 1;
		lock->owner_id = my_id;
	}
}

/* FIXME: check owner before decrementing depth?  return error? */

void h2_mutex_unlock(h2_mutex_t *lock)
{
#ifdef PARANOID
	unsigned int my_id = h2_thread_myid();
	if (lock->owner_id != my_id) {
	  printf("PANIC: h2_mutex_unlock by %08x != owner %08x\n", my_id, lock->owner_id);
	  exit(1);
	}
#endif
	/* Decrement Depth */
	lock->depth--;
	/* If lock no longer held, unlock mutex */
	if (lock->depth == 0) {
		lock->owner_id = 0;
		h2_plainmutex_unlock(&lock->mutex);
	}
}

int h2_mutex_trylock(h2_mutex_t *lock)
{
	unsigned int my_id = h2_thread_myid();
	if (h2_mutex_trylock(&lock->mutex) == 0) {
		/* Trylock succeded, set depth and owner */ 
		lock->depth = 1;
		lock->owner_id = my_id;
		return 0;
	} else if ((lock->type != H2_MUTEX_PLAIN) && (lock->owner_id == my_id)) {
		/* Trylock failed, but owner is me, so increment depth */
		lock->depth++;
		return 0;
	} else {
		/* trylock failed, I am not owner, return failure */
		return -1;
	}
}


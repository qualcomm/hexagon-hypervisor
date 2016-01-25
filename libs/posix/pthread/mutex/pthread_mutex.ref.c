/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * pthread_mutex.c
 * 
 * This file contains the implementation for a recursive mutex
 */

#include <pthread.h>
#include <c_std.h>

int pthread_mutex_lock(pthread_mutex_t *lock)
{
	unsigned int my_id = pthread_self();
	if (likely(pthread_plainmutex_trylock_id_np(&lock->mutex,my_id) == 0)) {
		/* Trylock succeeded, set depth and owner */
		lock->depth = 1;
		lock->owner_id = my_id;
		return 0;
	} else if ((lock->type != H2_MUTEX_PLAIN) && (lock->owner_id == my_id)) {
		/* Trylock failed, but owner is me, so increment depth */
		/* Should not get a race here because in unlock we clear owner_id before unlocking */
		lock->depth++;
		return 0;
	} else {
		/* Block until mutex is freed */
		pthread_plainmutex_lock_id_np(&lock->mutex,my_id);
		/* Lock was freed, set depth and owner */
		lock->depth = 1;
		lock->owner_id = my_id;
		return 0;
	}
}

/* FIXME: check owner before decrementing depth?  return error? */

int pthread_mutex_unlock(pthread_mutex_t *lock)
{
	unsigned int my_id = pthread_self();
#ifdef PARANOID
	if (lock->owner_id != my_id) {
	  printf("PANIC: pthread_mutex_unlock by %08x != owner %08x\n", my_id, lock->owner_id);
	  exit(1);
	}
#endif
	/* Decrement Depth */
	lock->depth--;
	/* If lock no longer held, unlock mutex */
	if (lock->depth == 0) {
		lock->owner_id = 0;
		pthread_plainmutex_unlock_id_np(&lock->mutex,my_id);
	}
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *lock)
{
	unsigned int my_id = pthread_self();
	if (likely(pthread_plainmutex_trylock_id_np(&lock->mutex,my_id) == 0)) {
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
		return EBUSY;
	}
}


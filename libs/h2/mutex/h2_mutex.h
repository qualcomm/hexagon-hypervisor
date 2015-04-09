/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_MUTEX_H
#define H2_MUTEX_H 1

/** @file h2_rmutex.h
 @brief Recursive Mutexes allow at most one thread to hold the mutex at a time, but allow a thread to lock the lock more than once.
*/
/** @addtogroup h2 
@{ */

#include <h2_plainmutex.h>

enum {
	H2_MUTEX_PLAIN = 0,
	H2_MUTEX_RECURSIVE = 1,
};

#define H2_MUTEX_T_INIT { H2_PLAINMUTEX_T_INIT, H2_MUTEX_PLAIN, 0, 0 }

/**
@brief Mutex Structure.  Please do not use directly 
*/
typedef struct {
	h2_plainmutex_t mutex;
	unsigned int type;
	unsigned int depth;
	unsigned int owner_id;
} __attribute__((aligned(8))) h2_mutex_t;

/**
Initialize a Mutex.  The mutex is initialized to be unheld.
@param[in] lock		Address of the Recursive Mutex
@returns None
@dependencies None
*/

static inline void h2_mutex_init_type(h2_mutex_t *lock, unsigned int type)
{
	h2_mutex_t temp = H2_MUTEX_T_INIT;
	temp.type = type;
	*lock = temp;
}

static inline void h2_mutex_init(h2_mutex_t *lock) { h2_mutex_init_type(lock,H2_MUTEX_PLAIN); }

/**
Lock a Mutex.  If the lock is held by another thread, this will block.
@param[in] lock		Address of the Recursive Mutex
@returns None for now, need to change to help POSIX
@dependencies None
*/
void h2_mutex_lock(h2_mutex_t *lock);

/**
Unlock a Mutex.  If the count of recursive locks is zero, a blocked thread will be woken.
@param[in] lock		Address of the Recursive Mutex
@returns None for now, need to change to help POSIX
@dependencies None
*/
void h2_mutex_unlock(h2_mutex_t *lock);

/**
Try to lock a Mutex.  If the mutex was already held by another thread, return failure.
@param[in] lock		Address of the Recursive Mutex
@returns 0 on success, nonzero otherwise
@dependencies None
*/
int h2_mutex_trylock(h2_mutex_t *lock);

/** @} */

#endif

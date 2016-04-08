/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PLAINMUTEX_H
#define H2_PLAINMUTEX_H 1

#include <pthread.h>

/** @file h2_mutex.h
 @brief Mutexes allow at most one thread to hold the mutex at a time
*/
/** @addtogroup h2 
@{ */

/** h2_mutex_t is a word */
typedef pthread_plainmutex_t h2_plainmutex_t;

/** H2_MUTEX_T_INIT is the value to initialize a mutex to */
#define H2_PLAINMUTEX_T_INIT PTHREAD_PLAINMUTEX_INITIALIZER_NP

/**
Initialize a mutex.  The mutex is not held once initialized.
@param[in] lock		Address of the mutex
@returns None
@dependencies None
*/
static inline void h2_plainmutex_init(h2_plainmutex_t *lock) { *lock = H2_PLAINMUTEX_T_INIT; }

/**
Lock a mutex.  If the mutex is already held, it will block until the mutex can be locked.
@param[in] lock		Address of the mutex
@returns None, but should return success/fail for POSIX integration
@dependencies None
*/

static inline void h2_plainmutex_lock(h2_plainmutex_t *lock) { pthread_plainmutex_lock_np(lock); }

/**
Unlock a mutex.
@param[in] lock		Address of the mutex
@returns None, but should return success/fail for POSIX integration
@dependencies None
*/
static inline void h2_plainmutex_unlock(h2_plainmutex_t *lock) { pthread_plainmutex_unlock_np(lock); }	/* unlock */

/**
Try to lock a mutex.  If the mutex is already held, return failure.
@param[in] lock		Address of the mutex
@returns 0 on success, nonzero value on failure.
@dependencies None
*/

static inline int h2_plainmutex_trylock(h2_plainmutex_t *lock) { return pthread_plainmutex_trylock_np(lock); }	/* just try... 0 if successful, nonzero if not */

/** @} */

#endif


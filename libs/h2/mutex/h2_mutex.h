/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_MUTEX_H
#define H2_MUTEX_H 1

/** @file h2_mutex.h
 @brief Mutexes allow at most one thread to hold the mutex at a time
*/
/** @addtogroup h2 
@{ */

/** h2_mutex_t is a word */
typedef unsigned int h2_mutex_t;

/** H2_MUTEX_T_INIT is the value to initialize a mutex to */
#define H2_MUTEX_T_INIT 0 

/**
Initialize a mutex.  The mutex is not held once initialized.
*/
static inline void h2_mutex_init(h2_mutex_t *lock) { *lock = H2_MUTEX_T_INIT; }

/**
Lock a mutex.  If the mutex is already held, it will block until the mutex can be locked.
@param[in] lock		Address of the mutex
@returns None, but should return success/fail for POSIX integration
@dependencies None
*/

void h2_mutex_lock(h2_mutex_t *lock);

/**
Unlock a mutex.
@param[in] lock		Address of the mutex
@returns None, but should return success/fail for POSIX integration
@dependencies None
*/
void h2_mutex_unlock(h2_mutex_t *lock);	/* unlock */

/**
Try to lock a mutex.  If the mutex is already held, return failure.
@param[in] lock		Address of the mutex
@returns 0 on success, nonzero value on failure.
@dependencies None
*/

int h2_mutex_trylock(h2_mutex_t *lock);	/* just try... 0 if successful, nonzero if not */

/** @} */

#endif


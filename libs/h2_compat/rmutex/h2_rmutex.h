/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_RMUTEX_H
#define H2_RMUTEX_H 1

/** @file h2_rmutex.h
 @brief Recursive Mutexes allow at most one thread to hold the mutex at a time, but allow a thread to lock the lock more than once.
*/
/** @addtogroup h2 
@{ */

#include <h2_mutex.h>

#define H2_RMUTEX_T_INIT { H2_PLAINMUTEX_T_INIT, H2_MUTEX_RECURSIVE, 0, 0 }

/**
@brief Mutex Structure.  Please do not use directly 
*/
typedef h2_mutex_t h2_rmutex_t;

/**
Initialize a Mutex.  The mutex is initialized to be unheld.
@param[in] lock		Address of the Recursive Mutex
@returns None
@dependencies None
*/
void h2_rmutex_init(h2_mutex_t *lock);

/**
Lock a Mutex.  If the lock is held by another thread, this will block.
@param[in] lock		Address of the Recursive Mutex
@returns None for now, need to change to help POSIX
@dependencies None
*/
void h2_rmutex_lock(h2_mutex_t *lock);

/**
Unlock a Mutex.  If the count of recursive locks is zero, a blocked thread will be woken.
@param[in] lock		Address of the Recursive Mutex
@returns None for now, need to change to help POSIX
@dependencies None
*/
void h2_rmutex_unlock(h2_mutex_t *lock);

/**
Try to lock a Mutex.  If the mutex was already held by another thread, return failure.
@param[in] lock		Address of the Recursive Mutex
@returns 0 on success, nonzero otherwise
@dependencies None
*/
int h2_rmutex_trylock(h2_mutex_t *lock);

/** @} */

#endif

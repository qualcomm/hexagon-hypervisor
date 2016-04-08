/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COND_H
#define H2_COND_H 1

/** @file h2_cond.h
 @brief Condition variables sleep until some condition changes.
*/
/** @addtogroup h2 
@{ */

#include <h2_mutex.h>
#include <h2_rmutex.h>
#include <pthread.h>

/** @brief Condition type definition.  Please do not access directly. */
typedef pthread_cond_t h2_cond_t;

/**
Initialize a condition variable.  
@param[in] cond		Address of the condition variable
@returns None
@dependencies None
*/

static inline void h2_cond_init(h2_cond_t *cond) { pthread_cond_init(cond,NULL); };

/**
Signal to one thread that a condition has changed.
@param[in] cond		Address of the condition variable
@returns None
@dependencies None
*/

static inline void h2_cond_signal(h2_cond_t *cond) { pthread_cond_signal(cond); };

/**
Signal to all threads that a condition has changed.
@param[in] cond		Address of the condition variable
@returns None
@dependencies None
*/

static inline void h2_cond_broadcast(h2_cond_t *cond) { pthread_cond_broadcast(cond); };

/**
Wait on a condition.  The mutex will be freed before blocking and re-acquired before returning.
@param[in] cond		Address of the condition variable
@param[in] mutex	Address of the mutex variable
@returns None
@dependencies None
*/

static inline void h2_cond_wait(h2_cond_t *cond, h2_mutex_t *mutex) { pthread_cond_wait(cond,mutex); };

/**
Wait on a condition.  The rmutex will be freed before blocking and re-acquired before returning.
@param[in] cond		Address of the condition variable
@param[in] rmutex	Address of the rmutex variable
@returns None
@dependencies None
*/
static inline void h2_cond_wait_rmutex(h2_cond_t *cond, h2_rmutex_t *mutex) { h2_cond_wait(cond,mutex); }

/** @} */

#endif


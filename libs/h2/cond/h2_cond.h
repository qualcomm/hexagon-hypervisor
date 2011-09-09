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

/** @brief Condition type definition.  Please do not access directly. */
typedef union {
	unsigned int raw;
	struct {
		unsigned short count;
		unsigned short n_waiting;
	};
} h2_cond_t;

/**
Initialize a condition variable.  
@param[in] cond		Address of the condition variable
@returns None
@dependencies None
*/

static inline void h2_cond_init(h2_cond_t *cond) { cond->raw = 0; };

/**
Signal to one thread that a condition has changed.
@param[in] cond		Address of the condition variable
@returns None
@dependencies None
*/

void h2_cond_signal(h2_cond_t *cond);

/**
Signal to all threads that a condition has changed.
@param[in] cond		Address of the condition variable
@returns None
@dependencies None
*/

void h2_cond_broadcast(h2_cond_t *cond);

/**
Wait on a condition.  The mutex will be freed before blocking and re-acquired before returning.
@param[in] cond		Address of the condition variable
@param[in] mutex	Address of the mutex variable
@returns None
@dependencies None
*/

void h2_cond_wait(h2_cond_t *cond, h2_mutex_t *mutex);

/**
Wait on a condition.  The rmutex will be freed before blocking and re-acquired before returning.
@param[in] cond		Address of the condition variable
@param[in] rmutex	Address of the rmutex variable
@returns None
@dependencies None
*/
void h2_cond_wait_rmutex(h2_cond_t *cond, h2_rmutex_t *mutex);

/** @} */

#endif


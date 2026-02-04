/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_SEM_H
#define H2_SEM_H 1

/** @file h2_sem.h
 @brief Counting semaphore
*/
/** @addtogroup h2 
@{ */

/* can't make the whole union volatile, c++ operator= unhappy */
/**
@brief Definition of the semaphore type.  Please do not use directly.
*/

#include <semaphore.h>
typedef sem_t h2_sem_t;

/**
Initialize a semaphore.  The semaphore is initialized to 1.
@param[in] sem		Address of the semaphore
@returns None
@dependencies None
*/
void h2_sem_init(h2_sem_t *sem);

/**
Initialize a semaphore with a specific value.
@param[in] sem		Address of the semaphore
@param[in] val		Value to initialize the semaphore with
@returns None
@dependencies None
*/
void h2_sem_init_val(h2_sem_t *sem, unsigned int val);

/**
Add to a semaphore.  If threads are blocked, they will be woken up.
@param[in] sem		Address of the semaphore
@param[in] amt		Amount to add to the semaphore
@returns Arbitrary value
@dependencies None
*/
int h2_sem_add(h2_sem_t *sem, unsigned int amt);

/**
Add one to a semaphore.
@param[in] sem		Address of the semaphore
@returns Arbitrary value
@dependencies None
*/
int h2_sem_up(h2_sem_t *sem);

/**
Decrement a semaphore.  If the semaphore is zero, block until it is positive.
@param[in] sem		Address of the semaphore
@returns Arbitrary value
@dependencies None
*/
int h2_sem_down(h2_sem_t *sem);

/**
Attempt to decrement a semaphore.  If unsuccessful, return failure.
@param[in] sem		Address of the semaphore
@returns 0 on success, nonzero otherwise
@dependencies None
*/
int h2_sem_trydown(h2_sem_t *sem);

/** @} */

#endif

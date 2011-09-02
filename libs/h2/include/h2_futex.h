/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_FUTEX_H
#define H2_FUTEX_H 1

/** @file h2_mutex.h
 * 
 * @brief Futex interfaces for use directly from C
 */

/** @addtogroup h2 
@{ */

/**
If the specified word is equal to val, attempt to block.
@note This function may fail to block spuriously.  The memory must be checked and, if necessary, additional requests to block may need to be made.
@param[in] lock		Address of a word in memory
@param[in] val		Expected value of the word in memory
@returns 0 on success, nonzero otherwise.
@dependencies None
*/

int h2_futex_wait(void *lock, int val);

/**
Attempt to wake up some number of threads waiting on a word.

@note This function may fail for temporary reasons.  If the operation fails, the caller should reread the word in memory and retry.

@param[in] lock		Address of a word in memory
@param[in] n_to_wake	Maximum number of threads to wake
@returns number of threads woken on success, negative value on error

@dependencies None
*/

int h2_futex_wake(void *lock, int n_to_wake);

/**
Block attempting to lock a priority-inversion-avoidance mutex

@note This function may fail for temporary reasons.  If the operation fails, the caller should reread the word in memory and retry.

@param[in] lock		Address of a word in memory
@returns 0 on success, negative value on failure

@dependencies None
*/

int h2_futex_lock_pi(void *lock);

/**
Unlock a priority-inversion-avoidance mutex

@note This function may fail for temporary reasons.  If the operation fails, the caller should reread the word in memory and retry.

@param[in] lock		Address of a word in memory
@returns 0 on success, negative value on failure

@dependencies None
*/
int h2_futex_unlock_pi(void *lock);

/** @} */

#endif


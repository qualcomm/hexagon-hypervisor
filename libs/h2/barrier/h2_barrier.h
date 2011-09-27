/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_BARRIER_H
#define H2_BARRIER_H 1

/** @file h2_barrier.h
 @brief Barrier waits for a certain number of threads to reach the same point before continuing
*/
/** @addtogroup h2 
@{ */

/** Return value for one of the threads at the barrier, arbitrarily */
#define H2_BARRIER_SERIAL_THREAD 1

/** Return value for all other threads at the barrier */
#define H2_BARRIER_OTHER 0

#ifndef ASM
#include <h2_mutex.h>

/** @brief Barrier type definition.  Please do not access directly. */
typedef union {
	struct {
		unsigned short count;
		unsigned short threads_left;
		unsigned int threads_total;
	};
	unsigned long long int raw;
} h2_barrier_t;

/**
Initialize a barrier.  No threads are waiting on the barrier.  The barrier is
configured to wait for threads_total threads to arrive.
@param[in] barrier	Address of the barrier to initialize
@param[in] threads_total	Number of threads to wait on at the barrier
@returns Zero on success, nonzero otherwise.
@dependencies None
*/

static inline int h2_barrier_init(h2_barrier_t *barrier, unsigned int threads_total)
{
	barrier->count = 0;
	barrier->threads_left = barrier->threads_total = threads_total;
	return 0;
}

/**
Wait at a barrier.  When the configured number of threads have arrived at the barrier, all will continue execution.
@param[in] barrier	Address of the barrier to initialize
@returns H2_BARRIER_SERIAL_THREAD for one of the threads, arbitrarily.  H2_BARRIER_OTHER is returned for all other threads.
@dependencies None
*/

int h2_barrier_wait(h2_barrier_t *barrier);

#endif

/** @} */

#endif


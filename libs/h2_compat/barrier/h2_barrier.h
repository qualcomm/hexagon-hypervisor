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

#include <h2_barrier_defs.h>
#include <h2_mutex.h>

/** @brief Barrier type definition.  Please do not access directly. */
typedef pthread_barrier_t h2_barrier_t;

/**
Initialize a barrier.  No threads are waiting on the barrier.  The barrier is
configured to wait for threads_total threads to arrive.
@param[in] barrier	Address of the barrier to initialize
@param[in] threads_total	Number of threads to wait on at the barrier
@param[in] spin_cycles  Number of cycles to spin before blocking.
@returns Zero on success, nonzero otherwise.
@dependencies None
*/

static inline int h2_barrier_init_spin(h2_barrier_t *barrier, unsigned short threads_total, unsigned short spin_cycles)
{
	return pthread_barrier_init_spin_np(barrier,threads_total,spin_cycles);
}

static inline int h2_barrier_init(h2_barrier_t *barrier, unsigned short threads_total)
{
	return h2_barrier_init_spin(barrier, threads_total, 0);
}

/**
Wait at a barrier.  When the configured number of threads have arrived at the barrier, all will continue execution.
@param[in] barrier	Address of the barrier to initialize
@returns H2_BARRIER_SERIAL_THREAD for one of the threads, arbitrarily.  H2_BARRIER_OTHER is returned for all other threads.
@dependencies None
*/

static inline int h2_barrier_wait(h2_barrier_t *barrier) { return pthread_barrier_wait(barrier); };

/** @} */

#endif


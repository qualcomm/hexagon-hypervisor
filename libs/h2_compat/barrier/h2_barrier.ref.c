/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_barrier.ref.c
 * @brief Barrier waits for a certain number of threads to reach the same point before continuing - Implementation
 */

#include "h2_barrier.h"

int h2_barrier_init_spin(h2_barrier_t *barrier, unsigned short threads_total, unsigned short spin_cycles)
{
	return pthread_barrier_init_spin_np(barrier,threads_total,spin_cycles);
}

int h2_barrier_init(h2_barrier_t *barrier, unsigned short threads_total)
{
	return h2_barrier_init_spin(barrier, threads_total, 0);
}

int h2_barrier_wait(h2_barrier_t *barrier) { 
	return pthread_barrier_wait(barrier); 
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>

int pthread_barrierattr_init(pthread_barrierattr_t *attr) { attr->unused = 0; return 0; }
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr) { (void)attr; return 0; }
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared) { (void)attr; (void)pshared; return 0; }
int pthread_barrierattr_getpshared(pthread_barrierattr_t *attr, int *pshared) { (void)attr; *pshared = PTHREAD_PROCESS_SHARED; return 0; }

int pthread_barrier_init_spin_np(pthread_barrier_t *barrier, unsigned short count, unsigned short spins)
{
	barrier->raw = 0;
	barrier->threads_left = barrier->threads_total = count;
	barrier->spins = spins;
	return 0;
}

int pthread_barrier_init(pthread_barrier_t *barrier, 
	const pthread_barrierattr_t *attr, 
	unsigned short count) 
{
	(void)attr;
	return pthread_barrier_init_spin_np(barrier,(unsigned short)count,0);
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) { (void)barrier; return 0; }

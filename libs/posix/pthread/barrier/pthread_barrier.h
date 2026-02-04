/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_BARRIER_H
#define PTHREAD_BARRIER_H 1
/* BARRIER */

#define PTHREAD_BARRIER_SERIAL_THREAD -1

#ifndef ASM
typedef struct { 
	int unused;
} pthread_barrierattr_t;

typedef union {
	struct {
		unsigned short threads_left;
		unsigned short version;
		unsigned short threads_total;
		unsigned short spins;
	};
	unsigned long long int raw;
} pthread_barrier_t;

int pthread_barrierattr_init(pthread_barrierattr_t *attr);
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);
int pthread_barrierattr_getpshared(pthread_barrierattr_t *attr, int *pshared);

int pthread_barrier_init_spin_np(pthread_barrier_t *barrier, unsigned short count, unsigned short spins);

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned short count) ;
int pthread_barrier_destroy(pthread_barrier_t *barrier);

int pthread_barrier_wait(pthread_barrier_t *barrier);
#endif /* ASM */
#endif

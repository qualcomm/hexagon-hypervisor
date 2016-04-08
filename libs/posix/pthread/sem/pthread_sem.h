/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_SEM_H
#define PTHREAD_SEM_H 1

/* This maybe isn't technically part of pthreads, but it should be. */
/* It's a similar primitive for us so we'll put it here */

#include <posix_time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
	unsigned int volatile raw;
	struct {
		unsigned short volatile val;
		unsigned short volatile n_waiting;
	};
} pthread_sem_t;

#define PTHREAD_SEM_VALUE_MAX_NP 0x7fff

static inline int pthread_sem_init_np(pthread_sem_t *sem, int pshared, unsigned int value)
{
	if (value > PTHREAD_SEM_VALUE_MAX_NP) value = PTHREAD_SEM_VALUE_MAX_NP;
	sem->raw = value;
	return 0;
}
static inline int pthread_sem_destroy_np(pthread_sem_t *sem) { return 0; }
int pthread_sem_wait_np(pthread_sem_t *sem);
static inline int pthread_sem_timedwait_np(pthread_sem_t *sem, const struct timespec *abstime) { return pthread_sem_wait_np(sem); }
int pthread_sem_trywait_np(pthread_sem_t *sem);
int pthread_sem_add_np(pthread_sem_t *sem, unsigned int amount);
static inline int pthread_sem_post_np(pthread_sem_t *sem) { return pthread_sem_add_np(sem,1); }
static inline int pthread_sem_getvalue_np(pthread_sem_t *sem, int *sval) { *sval = sem->val; return 0; }

#ifdef __cplusplus
} /* extern "C" */ 
#endif

#endif

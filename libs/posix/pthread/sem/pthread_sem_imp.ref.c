/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread_sem.h>

int pthread_sem_init_np(pthread_sem_t *sem, int pshared, unsigned int value)
{
	(void)pshared;
	if (value > PTHREAD_SEM_VALUE_MAX_NP) value = PTHREAD_SEM_VALUE_MAX_NP;
	sem->raw = value;
	return 0;
}
int pthread_sem_destroy_np(pthread_sem_t *sem) { (void)sem; return 0; }
int pthread_sem_timedwait_np(pthread_sem_t *sem, const struct timespec *abstime) { (void)abstime; return pthread_sem_wait_np(sem); }
int pthread_sem_post_np(pthread_sem_t *sem) { return pthread_sem_add_np(sem,1); }
int pthread_sem_getvalue_np(pthread_sem_t *sem, int *sval) { *sval = sem->val; return 0; }
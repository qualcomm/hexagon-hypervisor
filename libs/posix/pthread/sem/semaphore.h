/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H 1

/* This maybe isn't technically part of pthreads, but it should be. */
/* It's a similar primitive for us so we'll put it here */

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_sem_t sem_t;

static inline int sem_init(sem_t *sem, int pshared, unsigned int value) { return pthread_sem_init_np(sem,pshared,value); }
static inline int sem_destroy(sem_t *sem) { return 0; }
static inline int sem_wait(sem_t *sem) { return pthread_sem_wait_np(sem); }
static inline int sem_timedwait(sem_t *sem, const struct timespec *abstime) { return sem_wait(sem); }
static inline int sem_trywait(sem_t *sem) { return pthread_sem_trywait_np(sem); }
static inline int sem_add_np(sem_t *sem, unsigned int amount) { return pthread_sem_add_np(sem,amount); }
static inline int sem_post(sem_t *sem) { return sem_add_np(sem,1); }
static inline int sem_getvalue(sem_t *sem, int *sval) { return pthread_sem_getvalue_np(sem,sval); }

#ifdef __cplusplus
} /* extern "C" */ 
#endif

#endif

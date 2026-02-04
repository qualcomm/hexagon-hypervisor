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

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);
int sem_trywait(sem_t *sem);
int sem_add_np(sem_t *sem, unsigned int amount);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);

#ifdef __cplusplus
} /* extern "C" */ 
#endif

#endif

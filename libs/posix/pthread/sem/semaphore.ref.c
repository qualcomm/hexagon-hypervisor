/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <errno.h>
#include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value) { return pthread_sem_init_np(sem,pshared,value); }
int sem_destroy(sem_t *sem) { return 0; }
int sem_wait(sem_t *sem) { return pthread_sem_wait_np(sem); }
int sem_timedwait(sem_t *sem, const struct timespec *abstime) { return sem_wait(sem); }
int sem_trywait(sem_t *sem) { int r = pthread_sem_trywait_np(sem); if (r) { errno = r; return -1; } return 0; }
int sem_add_np(sem_t *sem, unsigned int amount) { return pthread_sem_add_np(sem,amount); }
int sem_post(sem_t *sem) { int r = pthread_sem_add_np(sem,1); if (r) { errno = r; return -1; } return 0; }
int sem_getvalue(sem_t *sem, int *sval) { return pthread_sem_getvalue_np(sem,sval); }
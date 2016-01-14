/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <qurt.h>

typedef qurt_sem_t sem_t;

struct timespec;

#define SEM_VALUE_MAX 0x7fff

static inline int sem_init(sem_t *sem, int pshared, unsigned int value) { sem->value.raw = (value & SEM_VALUE_MAX); return 0; }
static inline int sem_destroy(sem_t *sem) { return 0; }
static inline int sem_wait(sem_t *sem) { qurt_sem_down(sem); return 0; }
static inline int sem_timedwait(sem_t *sem, const struct timespec *abstime) { return qurt_sem_try_down(sem); }
static inline int sem_trywait(sem_t *sem) { return qurt_sem_try_down(sem); }
static inline int sem_post(sem_t *sem) { qurt_sem_up(sem); return 0; }
static inline int sem_getvalue(sem_t *sem, int *sval) { *sval = qurt_sem_get_val(sem); return 0; }
static inline int sem_open(const char *name, int oflag, ...) { return ENOTSUP; }
static inline int sem_close(sem_t *sem) { return ENOTSUP; }
static inline int sem_unlink(const char *name) { return ENOTSUP; }

#ifdef __cplusplus
} /* extern "C" */ 
#endif

#endif

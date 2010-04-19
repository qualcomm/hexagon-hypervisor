/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <sys/types.h>     // Get all C sys types - includes POSIX specific
#include <errno.h>     // error values

/* sem_t definition */
typedef unsigned int   sem_t;
#define _SEM_T

/* constant definitions */
#define SEM_FAILED       ((sem_t*) 0)
#define SEM_VALUE_MAX    ((unsigned int) 30) // If need be increase this

/* SUPPORTED API declarations */

/** \details
 * POSIX standard comes with two kinds of semaphores: named and unnamed
 * semaphores.
 *
 * This implementation of POSIX kernel API only provide unnamed semaphore.
 *
 * Named semaphore functions, including sem_open(), sem_close(), sem_unlink()
 * are not supported.
 *
 * sem_timedwait() is not provided.
 */

/** \defgroup semaphore POSIX Semaphore API */

/** \ingroup semaphore */
/** @{ */

/** Initialize an unnamed semaphore.
 * Please refer to POSIX standard for details.
 * @param pshared [in] This implementation does not support non-zero value, 
 * i.e., semaphore cannot be shared between processes in this implementation. 
 */                 
int sem_init(sem_t *sem, int pshared, unsigned int value);

/** Lock a semaphore.
 * Please refer to POSIX standard for details.
 */
int sem_wait(sem_t *sem);

/** Lock a semaphore.
 * Please refer to POSIX standard for details.
 */
int sem_trywait(sem_t *sem);

/** Unlock a semaphore.
 * Please refer to POSIX standard for details.
 */
int sem_post(sem_t *sem);

/** Get the value of a semaphore.
 * Please refer to POSIX standard for details.
 */
int sem_getvalue(sem_t *sem, int *value);

/** Destroy an unnamed semaphore.
 * Please refer to POSIX standard for details.
 */
int sem_destroy(sem_t *sem);

/** @} */

#if 0
// UNSUPPORTED API declarations
int    sem_timedwait(sem_t *__sem, const struct timespec *__abstime);
sem_t *sem_open(const char *__name, int __oflag, ...);
int    sem_close(sem_t *__sem);
int    sem_unlink(const char *__name);
#endif

#endif


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_RWLOCK_H
#define PTHREAD_RWLOCK_H 1

/*
 * Easiest is read-oriented lock.  Supports recursive readers.  Can starve writers.  
 * FIXME: later implement write-oriented lock.  It's trickier.
 * 
 * States:
 *  Unlocked
 *  One writer
 *  One writer with readers/writers waiting
 *  One or more readers
 *  One or more readers with writers waiting
 * Unlocked: 0
 * If any thread is waiting, read unlock wakes 1
 * If any thread is waiting, write unlock wakes all
 * 
 * Also keep total number of waiters to know when to call for wakeup
 */

typedef union {
	unsigned long long int raw;
	struct {
		unsigned int lockstate;
		unsigned int waiters;
	};
} pthread_rwlock_t;

typedef struct {
	unsigned int unused;
} pthread_rwlock_attr_t;

#define PTHREAD_RWLOCK_INITIALIZER { .raw = 0 }

static inline int pthread_condattr_init(pthread_condattr_t *attr) { attr->unused = 0; return 0; }
static inline int pthread_condattr_destroy(pthread_condattr_t *attr) { return 0; }
static inline int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }
static inline int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) { return 0; }

static inline int pthread_rwlock_init(pthread_rwlock_t *lock, pthread_rwlock_attr_t *attr) { lock->raw = 0; }
int pthread_rwlock_tryrdlock(pthread_rwlock_t *lock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *lock);
int pthread_rwlock_rdlock(pthread_rwlock_t *lock);
int pthread_rwlock_wrlock(pthread_rwlock_t *lock);
int pthread_rwlock_unlock(pthread_rwlock_t *lock);

#endif

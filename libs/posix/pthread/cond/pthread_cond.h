/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_COND_H
#define PTHREAD_COND_H 1
/* COND */
#include <time.h>
#include <pthread_misc.h>
#include <stdio.h>

typedef struct {
	int unused;
} pthread_condattr_t;

static inline int pthread_condattr_init(pthread_condattr_t *attr) { attr->unused = 0; return 0; }
static inline int pthread_condattr_destroy(pthread_condattr_t *attr) { return 0; }
static inline int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }
static inline int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) { return 0; }

typedef union {
	unsigned int raw;
	struct {
		unsigned short count;
		unsigned short n_waiting;
	};
} pthread_cond_t;
#define PTHREAD_COND_INITIALIZER { .raw = 0 }

static inline int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) { cond->raw = 0; return 0; }
static inline int pthread_cond_destroy(pthread_cond_t *cond) { return 0; }

int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

static inline int pthread_cond_timedwait(pthread_cond_t *cond, 
	pthread_mutex_t *mutex, 
	const struct timespec *abstime)
{ 
	/* EJP: FIXME: for now, just ignore the timeout.
	 * We should be able to fix this, we can request a new timer event and that will cause the futex to fail.
	 * Then we can check the time and if we haven't waited long enough we wait again.
	 * But we don't have the guest timer infrastructure just yet, so defer for now.
	 */
	unsigned long long int timeout;
	unsigned short basecount = cond->count;
	volatile unsigned short *vcount = &cond->count;
	int ret = ETIMEDOUT;
	/* Sigh. POSIX says we must fail if bad values in abstime */
	if (abstime->tv_nsec < 0) return EINVAL;
	if (abstime->tv_nsec >= 1000000000) return EINVAL;
	timeout = abstime->tv_sec;
	timeout <<= 30;
	timeout |= abstime->tv_nsec;
	pthread_mutex_unlock(mutex);
	while (h2_get_elapsed_nanos() < timeout) {
		if (basecount != *vcount) {
			ret = 0;
			break;
		}
		h2_yield();
	}
	pthread_mutex_lock(mutex);
	return ret;
}
#endif

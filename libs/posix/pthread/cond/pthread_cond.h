/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_COND_H
#define PTHREAD_COND_H 1
/* COND */

#include <posix_time.h>
#include <pthread_misc.h>
#include <stdio.h>

typedef struct {
	int unused;
} pthread_condattr_t;

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared);
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

typedef union {
	unsigned int raw;
	struct {
		unsigned short count;
		unsigned short n_waiting;
	};
} pthread_cond_t;
#define PTHREAD_COND_INITIALIZER { .raw = 0 }

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);

int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

#endif

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_MUTEX_H
#define PTHREAD_MUTEX_H 1

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK PTHREAD_MUTEX_NORMAL /* FIXME: make this work */
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_FAST_NP PTHREAD_MUTEX_NORMAL

#ifndef ASM
typedef struct {
	pthread_plainmutex_t mutex;
	unsigned int type;
	unsigned int depth;
	unsigned int owner_id;
} __attribute__((aligned(8))) pthread_mutex_t;
#else
#define PLAINMUTEX_OFFSET 0
#define TYPE_OFFSET 4
#define DEPTH_OFFSET 8
#define OWNER_ID_OFFSET 12
#define OWNER_ID_DEPTH_OFFSET 8
#define TYPE_PLAINMUTEX_OFFSET 0
#if PLAINMUTEX_OFFSET != 0
#error KEEP PLAINMUTEX AT OFFSET ZERO!
#endif 
#endif

#ifndef ASM
#include <pthread_plainmutex.h>

/* MUTEX */

typedef struct {
	unsigned int type;
	/* Don't need PSHARED ? */
} pthread_mutexattr_t;

#define PTHREAD_MUTEX_INITIALIZER { PTHREAD_PLAINMUTEX_INITIALIZER_NP, PTHREAD_MUTEX_NORMAL, 0, 0 }
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP { PTHREAD_PLAINMUTEX_INITIALIZER_NP, PTHREAD_MUTEX_RECURSIVE, 0, 0 }
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP { PTHREAD_PLAINMUTEX_INITIALIZER_NP, PTHREAD_MUTEX_ERRORCHECK, 0, 0 }

static inline int pthread_mutexattr_init(pthread_mutexattr_t *attr) { attr->type = PTHREAD_MUTEX_NORMAL; return 0; }
static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) { return 0; }
static inline int pthread_mutexattr_gettype(pthread_mutexattr_t *attr, int *type) { *type = attr->type; return 0; }
static inline int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) { attr->type = type; return 0; }
static inline int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }
static inline int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared) { return 0; }
static inline int pthread_mutexattr_getprotocol(pthread_mutexattr_t *attr, int *protocol) { *protocol = PTHREAD_PRIO_NONE; return 0; }
static inline int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol) { return 0; }
static inline int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *attr, int *ceil) { *ceil = 0; return 0; }
static inline int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int ceil) { return 0; }

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex) { return 0; }
static inline int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	pthread_mutex_t default_mutex = PTHREAD_MUTEX_INITIALIZER;
	if (attr) default_mutex.type = attr->type;
	*mutex = default_mutex;
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

static inline int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int ceil, int *oldceil) { *oldceil = 0; return 0; }
static inline int pthread_mutex_getprioceiling(pthread_mutex_t *mutex, int *ceil)  { *ceil = 0; return 0; }

#endif

#endif

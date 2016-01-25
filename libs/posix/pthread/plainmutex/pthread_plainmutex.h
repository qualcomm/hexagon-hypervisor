/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_PLAINMUTEX_H
#define PTHREAD_PLAINMUTEX_H 1

#include <pthread.h>
#include <h2_common_c_std.h>

typedef unsigned int pthread_plainmutex_t;

#define PTHREAD_PLAINMUTEX_INITIALIZER_NP 0 

static inline void pthread_plainmutex_init_np(h2_plainmutex_t *lock) { *lock = PTHREAD_PLAINMUTEX_INITIALIZER_NP; }

int pthread_plainmutex_lock_id_canfail_np(pthread_plainmutex_t *lock, pthread_t id);
static inline int pthread_plainmutex_lock_id_np(pthread_plainmutex_t *lock, pthread_t id)
{
	while (unlikely(pthread_plainmutex_lock_id_canfail_np(lock,id) != 0)) /* TRY AGAIN */;
	return 0;
}
static inline int pthread_plainmutex_lock_np(pthread_plainmutex_t *lock) { return pthread_plainmutex_lock_id_np(lock,pthread_self()); }

int pthread_plainmutex_unlock_id_canfail_np(pthread_plainmutex_t *lock, pthread_t id);
static inline int pthread_plainmutex_unlock_id_np(pthread_plainmutex_t *lock, pthread_t id)
{
	while (unlikely(pthread_plainmutex_unlock_id_canfail_np(lock,id) != 0)) /* TRY AGAIN */;
	return 0;
}
static inline int pthread_plainmutex_unlock_np(pthread_plainmutex_t *lock) { return pthread_plainmutex_unlock_id_np(lock,pthread_self()); }

int pthread_plainmutex_trylock_id_np(pthread_plainmutex_t *lock, pthread_t id);
static inline int pthread_plainmutex_trylock_np(pthread_plainmutex_t *lock) { return pthread_plainmutex_trylock_id_np(lock,pthread_self()); }

#endif


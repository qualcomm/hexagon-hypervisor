/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <pthread_plainmutex.h>

#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

void pthread_plainmutex_init_np(pthread_plainmutex_t *lock) { *lock = PTHREAD_PLAINMUTEX_INITIALIZER_NP; }
int pthread_plainmutex_lock_id_np(pthread_plainmutex_t *lock, pthread_t id)
{
	while (unlikely(pthread_plainmutex_lock_id_canfail_np(lock,id) != 0)) /* TRY AGAIN */;
	return 0;
}
int pthread_plainmutex_lock_np(pthread_plainmutex_t *lock) { return pthread_plainmutex_lock_id_np(lock,pthread_self()); }
int pthread_plainmutex_unlock_id_np(pthread_plainmutex_t *lock, pthread_t id)
{
	while (unlikely(pthread_plainmutex_unlock_id_canfail_np(lock,id) != 0)) /* TRY AGAIN */;
	return 0;
}
int pthread_plainmutex_unlock_np(pthread_plainmutex_t *lock) { return pthread_plainmutex_unlock_id_np(lock,pthread_self()); }
int pthread_plainmutex_trylock_np(pthread_plainmutex_t *lock) { return pthread_plainmutex_trylock_id_np(lock,pthread_self()); }

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_plainmutex.ref.c
 * @brief Plain Mutexes allow at most one thread to hold the mutex at a time - Implementation
 */

#include "h2_plainmutex.h"

void h2_plainmutex_init(h2_plainmutex_t *lock) { 
	*lock = H2_PLAINMUTEX_T_INIT; 
}

void h2_plainmutex_lock(h2_plainmutex_t *lock) { 
	pthread_plainmutex_lock_np(lock); 
}

void h2_plainmutex_unlock(h2_plainmutex_t *lock) { 
	pthread_plainmutex_unlock_np(lock); 
}

int h2_plainmutex_trylock(h2_plainmutex_t *lock) { 
	return pthread_plainmutex_trylock_np(lock); 
}

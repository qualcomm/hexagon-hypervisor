/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_rmutex.ref.c
 * @brief Recursive Mutexes allow at most one thread to hold the mutex at a time - Implementation
 */

#include "h2_rmutex.h"

void h2_rmutex_init(h2_mutex_t *lock) { 
	h2_mutex_init_type(lock,H2_MUTEX_RECURSIVE); 
}

void h2_rmutex_lock(h2_mutex_t *lock) { 
	h2_mutex_lock(lock); 
}

void h2_rmutex_unlock(h2_mutex_t *lock) { 
	h2_mutex_unlock(lock); 
}

int h2_rmutex_trylock(h2_mutex_t *lock) { 
	return h2_mutex_trylock(lock); 
}

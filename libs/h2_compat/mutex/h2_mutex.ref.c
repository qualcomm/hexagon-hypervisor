/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_mutex.ref.c
 * @brief Mutexes allow at most one thread to hold the mutex at a time - Implementation
 */

#include "h2_mutex.h"

void h2_mutex_init_type(h2_mutex_t *lock, unsigned int type)
{
	h2_mutex_t temp = H2_MUTEX_T_INIT;
	temp.type = type;
	*lock = temp;
}

void h2_mutex_init(h2_mutex_t *lock) { 
	h2_mutex_init_type(lock,H2_MUTEX_PLAIN); 
}

void h2_mutex_lock(h2_mutex_t *lock) { 
	pthread_mutex_lock(lock); 
}

void h2_mutex_unlock(h2_mutex_t *lock) { 
	pthread_mutex_unlock(lock); 
}

int h2_mutex_trylock(h2_mutex_t *lock) { 
	return pthread_mutex_trylock(lock); 
}

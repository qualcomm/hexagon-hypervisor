/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_cond.ref.c
 * @brief Condition variables sleep until some condition changes - Implementation
 */

#include "h2_cond.h"

void h2_cond_init(h2_cond_t *cond) { 
	pthread_cond_init(cond,NULL); 
}

void h2_cond_signal(h2_cond_t *cond) { 
	pthread_cond_signal(cond); 
}

void h2_cond_broadcast(h2_cond_t *cond) { 
	pthread_cond_broadcast(cond); 
}

void h2_cond_wait(h2_cond_t *cond, h2_mutex_t *mutex) { 
	pthread_cond_wait(cond,mutex); 
}

void h2_cond_wait_rmutex(h2_cond_t *cond, h2_rmutex_t *mutex) { 
	h2_cond_wait(cond,mutex); 
}

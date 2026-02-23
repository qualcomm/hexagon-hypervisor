/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_sem.ref.c
 * @brief Counting semaphore - Implementation
 */

#include "h2_sem.h"

void h2_sem_init(h2_sem_t *sem) { 
	sem_init(sem,1,1); 
}

void h2_sem_init_val(h2_sem_t *sem, unsigned int val) { 
	sem_init(sem,1,val); 
}

int h2_sem_add(h2_sem_t *sem, unsigned int amt) { 
	return sem_add_np(sem,amt); 
}

int h2_sem_up(h2_sem_t *sem) { 
	return sem_post(sem); 
}

int h2_sem_down(h2_sem_t *sem) { 
	return sem_wait(sem); 
}

int h2_sem_trydown(h2_sem_t *sem) { 
	return sem_trywait(sem); 
}

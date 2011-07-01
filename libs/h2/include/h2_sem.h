/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_SEM_H
#define H2_SEM_H 1

/* can't make the whole union volatile, c++ operator= unhappy */
typedef union {
	unsigned int volatile raw;
	struct {
		unsigned short volatile val;
		unsigned short volatile n_waiting;
	};
} h2_sem_t;

/*  Sets the 'raw' value  */
#define H2_SEM_T_INIT { 1 }

int h2_sem_add(h2_sem_t *sem, unsigned int amt);
static inline int h2_sem_up(h2_sem_t *sem) { return h2_sem_add(sem,1); };
int h2_sem_down(h2_sem_t *sem);
int h2_sem_trydown(h2_sem_t *sem);
static inline void h2_sem_init(h2_sem_t *sem) { h2_sem_t temp = H2_SEM_T_INIT; *sem = temp; };
static inline void h2_sem_init_val(h2_sem_t *sem, unsigned short val) { sem->raw = val; };

#endif


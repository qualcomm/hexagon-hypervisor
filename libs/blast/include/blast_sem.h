/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_SEM_H
#define BLAST_SEM_H 1

typedef union {
	unsigned int raw;
	struct {
		unsigned short val;
		unsigned short n_waiting;
	};
} blast_sem_t;

int blast_sem_add(blast_sem_t *sem, unsigned int amt);
static inline int blast_sem_up(blast_sem_t *sem) { return blast_sem_add(sem,1); };
int blast_sem_down(blast_sem_t *sem);
int blast_sem_trydown(blast_sem_t *sem);
static inline void blast_sem_init(blast_sem_t *sem) { sem->raw = 1; };
static inline void blast_sem_init_val(blast_sem_t *sem, unsigned short val) { sem->raw = val; };

#endif


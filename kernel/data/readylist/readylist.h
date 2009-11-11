/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef READYLIST_H
#define READYLIST_H 1

#include <context.h>
#include <max.h>

extern BLASTK_thread_context *BLASTK_ready[MAX_PRIOS];
extern unsigned int BLASTK_ready_valids;

static inline int BLASTK_ready_best_prio()
{
	return Q6_R_ct0_R(BLASTK_ready_valids);
}

static inline void BLASTK_ready_append(BLASTK_thread_context *thread)
{
	int prio = thread->prio;
	BLASTK_ring_append(&BLASTK_ready[prio],thread);
	BLASTK_ready_valids |= 1<<prio;
}

static inline void BLASTK_ready_insert(BLASTK_thread_context *thread)
{
	int prio = thread->prio;
	BLASTK_ring_insert(&BLASTK_ready[prio],thread);
	BLASTK_ready_valids |= 1<<prio;
}

static inline void BLASTK_ready_remove(BLASTK_thread_context *thread)
{
	int prio = thread->prio;
	BLASTK_ring_remove(&BLASTK_ready[prio],thread);
	if (BLASTK_ready[prio] == NULL) BLASTK_ready_valids = Q6_R_clrbit_RR(BLASTK_ready_valids,prio);
}

static inline BLASTK_thread_context *BLASTK_ready_getbest()
{
	BLASTK_thread_context *ret;
	int prio;
	if ((BLASTK_ready_valids) == 0) return NULL;
	prio = BLASTK_ready_best_prio();
	ret = BLASTK_ready[prio];
	BLASTK_ready_remove(ret);
	return ret;
}

#endif


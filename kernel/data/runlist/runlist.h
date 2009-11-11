/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef RUNLIST_H
#define RUNLIST_H 1

#include <context.h>
#include <max.h>

extern BLASTK_thread_context *BLASTK_runlist[MAX_PRIOS];
extern unsigned int BLASTK_runlist_valids;

static inline void BLASTK_runlist_push(BLASTK_thread_context *newthread)
{
	unsigned int prio = newthread->prio;
	newthread->next = BLASTK_runlist[prio];
	BLASTK_runlist[prio] = newthread;
	BLASTK_runlist_valids |= 1<<prio;
}

static inline int BLASTK_runlist_worst_prio()
{
	return (8*sizeof(BLASTK_runlist_valids)-1-Q6_R_cl0_R(BLASTK_runlist_valids);
}

static inline void runlist_remove(BLASTK_thread_context *thread)
{
	BLASTK_thread_context *tmp;
	int prio = thread->prio;
	if (BLASTK_runlist[prio] == thread) {
		BLASTK_runlist[prio] = thread->next;
	} else {
		for (tmp = BLASTK_runlist[thread->prio]; tmp->next != thread; tmp = tmp->next) {
			/* Look for thread in list */
		}
		tmp->next = thread->next;
	}
	if (BLASTK_runlist[prio] == NULL) {
		BLASTK_runlist_valids ^= 1<<prio;
	}
}

#endif


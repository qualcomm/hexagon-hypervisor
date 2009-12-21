/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef RUNLIST_H
#define RUNLIST_H 1

#include <context.h>
#include <max.h>
#include <q6protos.h>

extern H2K_thread_context *H2K_runlist[MAX_PRIOS] IN_SECTION(".data.sched.runlist");
extern u32_t H2K_runlist_valids IN_SECTION(".data.sched.runlist");

static inline void H2K_runlist_push(H2K_thread_context *newthread)
{
	u32_t prio = newthread->prio;
	newthread->next = H2K_runlist[prio];
	H2K_runlist[prio] = newthread;
	H2K_runlist_valids |= 1<<prio;
}

static inline u32_t H2K_runlist_worst_prio()
{
	return ((8*sizeof(H2K_runlist_valids))-1)-Q6_R_cl0_R(H2K_runlist_valids);
}

static inline void H2K_runlist_remove(H2K_thread_context *thread)
{
	H2K_thread_context *tmp;
	u32_t prio = thread->prio;
	if (H2K_runlist[prio] == thread) {
		H2K_runlist[prio] = thread->next;
	} else {
		for (tmp = H2K_runlist[thread->prio]; tmp->next != thread; tmp = tmp->next) {
			/* Look for thread in list */
		}
		tmp->next = thread->next;
	}
	if (H2K_runlist[prio] == NULL) {
		H2K_runlist_valids ^= 1<<prio;
	}
}

void H2K_runlist_init(void);

#endif


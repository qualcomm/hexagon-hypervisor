/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef READYLIST_H
#define READYLIST_H 1

#include <context.h>
#include <ring.h>
#include <q6protos.h>
#include <max.h>

extern H2K_thread_context *H2K_ready[MAX_PRIOS] IN_SECTION(".data.sched.ready");
extern u32_t H2K_ready_valids IN_SECTION(".data.sched.ready");

static inline int H2K_ready_best_prio()
{
	return Q6_R_ct0_R(H2K_ready_valids);
}

static inline void H2K_ready_append(H2K_thread_context *thread)
{
	u32_t prio = thread->prio;
	H2K_ring_append(&H2K_ready[prio],thread);
	H2K_ready_valids |= 1<<prio;
}

static inline void H2K_ready_insert(H2K_thread_context *thread)
{
	u32_t prio = thread->prio;
	H2K_ring_insert(&H2K_ready[prio],thread);
	H2K_ready_valids |= 1<<prio;
}

static inline void H2K_ready_remove(H2K_thread_context *thread)
{
	u32_t prio = thread->prio;
	H2K_ring_remove(&H2K_ready[prio],thread);
	if (H2K_ready[prio] == NULL) H2K_ready_valids = Q6_R_clrbit_RR(H2K_ready_valids,prio);
}

static inline H2K_thread_context *H2K_ready_getbest()
{
	H2K_thread_context *ret;
	u32_t prio;
	if ((H2K_ready_valids) == 0) return NULL;
	prio = H2K_ready_best_prio();
	ret = H2K_ready[prio];
	H2K_ready_remove(ret);
	return ret;
}

void H2K_readylist_init(void);

#endif


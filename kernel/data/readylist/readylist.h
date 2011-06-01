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
#include <globals.h>

/* Get the best ready priority */
static inline u32_t H2K_ready_best_prio()
{
	u32_t prio = 0;
	u32_t ct0;
	u32_t i;
	for (i = 0; i < MAX_PRIOS/64; i++) {
		ct0 = Q6_R_ct0_R((u32_t) H2K_gp->ready_valids[i]);
		prio += ct0;
		if (ct0 < 32) return prio;
		ct0 = Q6_R_ct0_R((u32_t) (H2K_gp->ready_valids[i] >> 32));
		prio += ct0;
		if (ct0 < 32) return prio;
	}
	return prio;
}

/* Check whether any threads are ready */
static inline u32_t H2K_ready_any_valid()
{
	return H2K_ready_best_prio() < MAX_PRIOS;
}

/* Check whether a thread at a given priority is ready */
static inline u32_t H2K_ready_prio_valid(u32_t prio)
{
	if (prio > MAX_PRIO) return 0;
	return (H2K_gp->ready_valids[prio >> 6] & (1ULL << (prio & 0x3f))) != 0;
}

/* Set a given priority as valid */
static inline void H2K_ready_set_prio(u32_t prio)
{
	H2K_gp->ready_valids[prio >> 6] |= (1ULL << (prio & 0x3f));
}

/* Set a given priority as invalid */
static inline void H2K_ready_clear_prio(u32_t prio)
{
	H2K_gp->ready_valids[prio >> 6] &= ~(1ULL << (prio & 0x3f));
}

/* Take the thread and place it in the ready structure,
 * the last one to be scheduled at its priority */
static inline void H2K_ready_append(H2K_thread_context *thread)
{
	u32_t prio = thread->prio;
	thread->status = H2K_STATUS_READY;
	H2K_ring_append(&H2K_gp->ready[prio],thread);
	H2K_ready_set_prio(prio);
}

/* Take the thread and place it in the ready structure,
 * the first thread to be scheduled at its priority */
static inline void H2K_ready_insert(H2K_thread_context *thread)
{
	u32_t prio = thread->prio;
	thread->status = H2K_STATUS_READY;
	H2K_ring_insert(&H2K_gp->ready[prio],thread);
	H2K_ready_set_prio(prio);
}

/* Remove a specific thread from the ready list */
/* The caller guarantees that the thread is actually in the ready list correctly */
static inline void H2K_ready_remove(H2K_thread_context *thread)
{
	u32_t prio = thread->prio;
	H2K_ring_remove(&H2K_gp->ready[prio],thread);
	if (H2K_gp->ready[prio] == NULL) H2K_ready_clear_prio(prio);
}

/* Remove and return the best thread */
static inline H2K_thread_context *H2K_ready_getbest()
{
	H2K_thread_context *ret;
	u32_t prio;
	if (!H2K_ready_any_valid()) return NULL;
	prio = H2K_ready_best_prio();
	ret = H2K_gp->ready[prio];
	H2K_ready_remove(ret);
	return ret;
}

void H2K_readylist_init(void) IN_SECTION(".text.init.readylist");

#endif

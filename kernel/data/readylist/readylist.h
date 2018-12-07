/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef READYLIST_H
#define READYLIST_H 1

#include <context.h>
#include <ring.h>
#include <hexagon_protos.h>
#include <max.h>
#include <globals.h>
#include <hw.h>
#include <log.h>

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

/* Return head of ready list at prio */
static inline H2K_thread_context *H2K_ready_head(u32_t prio, u32_t hthread) {
	H2K_thread_context *head = H2K_gp->ready[prio];
	H2K_thread_context *ret = head;

#ifdef CLUSTER_SCHED_HACK
	u32_t hthread_xe = ((H2K_get_ssr() & SSR_XE_BIT_MASK) != 0);
	u32_t cluster = H2K_hthread_cluster(hthread);

	H2K_log("hthread %d  cluster %d  xe_set %d\n", hthread, cluster, H2K_gp->xe_set[cluster]);

	/* Skip threads that have xe set if that would increase the total xe threads
		 per cluster beyond the limit */
	if (!hthread_xe	&& (H2K_gp->xe_set[cluster] == MAX_HVX_PER_CLUSTER)) {
		while ((ret != NULL) && (ret->ssr & SSR_XE_BIT_MASK)) {  // xe set in new thread
			H2K_log("\tSkipping hvx thread in H2K_ready_head\n");
			ret = (H2K_thread_context *)H2K_ring_next(head, ret);  // try the next one
		}
	}
	/* FIXME: Check next lower priority if we don't find a non-xe thread here?
		 Probably not, since scheduling a lower-priority thread will cause check_sanity
		 to raise the resched interrupt immediately, and we'll be right back here */

	/* This is dicey to say the least. No guarantee that the resched interrupt
		 will go to the other cluster. In fact, if all the hw threads on the other
		 cluster are busy, then we won't make any progress. */
	if (NULL == ret) {  // didn't find anything to schedule
		H2K_log("\tDidn't find a thread to schedule\n");

		/* If we are returing NULL, then we must have gone through the above loop, and therefore
			 hthread_xe must be false, so we don't need the code below */
		/* if (hthread_xe) { */
		/* 	H2K_gp->xe_set[cluster]--; */
		/* 	H2K_set_ssr(H2K_get_ssr() & ~SSR_XE_BIT_MASK); */
		/* } */
		resched_int();    // try to get another thread to pick up what we skipped
	} else {
		if (!hthread_xe && (ret->ssr & SSR_XE_BIT_MASK)) {  // new hthread with xe set
			H2K_gp->xe_set[cluster]++;
			H2K_log("\tNow xe_set++ == %d\n", H2K_gp->xe_set[cluster]);
		}
		if (hthread_xe && !(ret->ssr & SSR_XE_BIT_MASK)) {
			H2K_gp->xe_set[cluster]--;
			H2K_log("\tNow xe_set-- == %d\n", H2K_gp->xe_set[cluster]);
		}
	}
#endif

	return ret;
}

/* Remove and return the best thread */
/* me == thread being switched out */
static inline H2K_thread_context *H2K_ready_getbest(u32_t hthread)
{
	H2K_thread_context *ret;
	u32_t prio;
	prio = H2K_ready_best_prio();
	if (prio >= MAX_PRIOS) {  // !H2K_ready_any_valid(), go to sleep
#ifdef CLUSTER_SCHED_HACK
		u32_t hthread_xe = ((H2K_get_ssr() & SSR_XE_BIT_MASK) != 0);
		u32_t cluster = H2K_hthread_cluster(hthread);

		if (hthread_xe) {
			H2K_gp->xe_set[cluster]--;
			H2K_set_ssr(H2K_get_ssr() & ~SSR_XE_BIT_MASK);
			H2K_log("getbest: hthread %d  cluster %d  xe_set-- == %d\n", hthread, cluster, H2K_gp->xe_set[cluster]);
		}
#endif
		return NULL;
	}

	ret = H2K_ready_head(prio, hthread);
	if (ret != NULL) {
		H2K_ready_remove(ret);
	}
	return ret;
}

void H2K_readylist_init(void) IN_SECTION(".text.init.readylist");

#endif

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
#include <runlist.h>

/* Get the best ready priority */
static inline u32_t H2K_ready_best_prio()
{
	u32_t prio = 0;
	u32_t ct0;
	u32_t i;
	for (i = 0; i < MAX_PRIOS/64; i++) {
		ct0 = (u32_t)Q6_R_ct0_R((u32_t) H2K_gp->ready_valids[i]);
		prio += ct0;
		if (ct0 < 32) return prio;
		ct0 = (u32_t)Q6_R_ct0_R((u32_t) (H2K_gp->ready_valids[i] >> 32));
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

#ifdef CLUSTER_SCHED
static inline u32_t H2K_hthread_cluster(u32_t hthread) {
	return (hthread / H2K_gp->cluster_hthreads);
}
#endif

/* Return head of ready list at prio */
static inline H2K_thread_context *H2K_ready_head(u32_t prio, u32_t hthread) {
	H2K_thread_context *head = H2K_gp->ready[prio];
	H2K_thread_context *ret = head;

	if (NULL == ret) {
		return ret;
	}

#ifdef CLUSTER_SCHED
	if (!H2K_gp->cluster_sched) {
		return ret;
	}

	u32_t ssr = H2K_get_ssr();
	u32_t hthread_xe = ssr & SSR_XE_BIT_MASK;
	u32_t hthread_xe2 = ssr & SSR_XE2_BIT_MASK;
	u32_t cluster = H2K_hthread_cluster(hthread);  // cluster of hthread
	u32_t coprocs = XE_SET_COUNT(cluster) + XE2_SET_COUNT(cluster);  // coproc threads in cluster
	u32_t hthreads;  // candidate hthreads in other clusters to interrupt
	s32_t victim;  // hthread to receive interrupt
	u32_t head_xe, head_xe2;  // thread at head needs xe/xe2
	u32_t head_coprocs;  // number of coprocs needed by thread at head
	u32_t i;

	H2K_log("hthread %d  cluster %d  coprocs %d  hthread_xe %d  hthread_xe2 %d\n", hthread, cluster, coprocs, hthread_xe, hthread_xe2);
	H2K_log("coprocs_max %d\n", H2K_gp->coproc_max);
	H2K_log("cluster_masks  0x%08x 0x%08x 0x%08x 0x%08x\n", H2K_gp->cluster_mask[0], H2K_gp->cluster_mask[1], H2K_gp->cluster_mask[2], H2K_gp->cluster_mask[3]);

	H2K_log("ready_head 1: hthread %d  xe_set 0x%08x\n", hthread, H2K_gp->xe_set);
	H2K_log("ready_head 1: hthread %d  xe2_set 0x%08x\n", hthread, H2K_gp->xe2_set);

	/* Skip threads that have xe/xe2 set if that would increase the total xe+xe2 threads
		 per cluster beyond the limit */
	if (coprocs == H2K_gp->coproc_max || coprocs == H2K_gp->coproc_max - 1) {  // cluster at/near limit
		head_xe = ret->ssr & SSR_XE_BIT_MASK;
		head_xe2 = ret->ssr & SSR_XE2_BIT_MASK;
		while (NULL != ret) {
			H2K_log("hthread %d head_xe %d\n", ((ret->ssr & SSR_XE_BIT_MASK) == 1));
			H2K_log("hthread %d head_xe2 %d\n", ((ret->ssr & SSR_XE2_BIT_MASK) ==1));
			/* If this hthread doesn't already hold xe/xe2 but new thread needs it */
			if (((coprocs == H2K_gp->coproc_max)  // and need 1 new coproc
					 && ((!hthread_xe && (ret->ssr & SSR_XE_BIT_MASK))
							 != (!hthread_xe2 && (ret->ssr & SSR_XE2_BIT_MASK))))
					|| ((coprocs == H2K_gp->coproc_max - 1)  // and need 2 new coprocs
							&& ((!hthread_xe && (ret->ssr & SSR_XE_BIT_MASK))
									&& (!hthread_xe2 && (ret->ssr & SSR_XE2_BIT_MASK))))) {
				H2K_log("\ththread %d Skipping hvx/hmx thread in H2K_ready_head\n", hthread);
				ret = (H2K_thread_context *)H2K_ring_next(head, ret);  // try the next one
			} else {
				break;
			}
		}
	}
	if (NULL == ret) {  // didn't find anything to schedule
		H2K_log("\ththread %d Didn't find a thread to schedule\n", hthread);
		H2K_log("\ththread %d Other clusters xe_set 0x%08x\n", hthread, H2K_gp->xe_set & ~(H2K_gp->cluster_mask[cluster]));
		H2K_log("\ththread %d Other clusters xe2_set 0x%08x\n", hthread, H2K_gp->xe2_set & ~(H2K_gp->cluster_mask[cluster]));

		/* This hthread is no longer using xe/xe2 */
		if (hthread_xe) {
			XE_SET_CLR(hthread);
			ssr &= ~SSR_XE_BIT_MASK;
			H2K_log("ready_head 2: hthread %d  xe_set 0x%08x\n", hthread, H2K_gp->xe_set);
		}
		if (hthread_xe2) {
			XE2_SET_CLR(hthread);
			ssr &= ~SSR_XE2_BIT_MASK;
			H2K_log("ready_head 2: hthread %d  xe2_set 0x%08x\n", hthread, H2K_gp->xe2_set);
		}
		H2K_set_ssr(ssr);

		/* Try to interrupt a thread of equal or lower priority on the other clusters that doesn't have xe/xe2 set */
		/* If the thread at the head of the ready list needs both coprocs, only choose clusters that have 2 slots free */
		head_coprocs = (head_xe != 0) + (head_xe2 != 0);
		hthreads = 0;
		for (i = 0; i < H2K_gp->cluster_clusters; i++) {
			if (H2K_gp->coproc_max - XE_SET_COUNT(i) >= head_coprocs) {  // has room
				hthreads |= H2K_gp->cluster_mask[i];
			}
		}
		victim = H2K_runlist_prio_hthreads(hthreads, prio);

		if (victim != -1) {  // any eligible
			H2K_log("\ththread %d Signal thread %d\n", hthread, victim);
			iassignw(CLUSTER_RESCHED_INT, ~(0x1 << victim));  // steer the interrupt
			cluster_resched_int();    // try to get another thread to pick up what we skipped
		}
	} else {
		if (!hthread_xe && (ret->ssr & SSR_XE_BIT_MASK)) {  // new hthread with xe set
			XE_SET_SET(hthread);
			H2K_log("\ththread %d Now set xe_set to  0x%08x\n", hthread, H2K_gp->xe_set);
		}
		if (hthread_xe && !(ret->ssr & SSR_XE_BIT_MASK)) {
			XE_SET_CLR(hthread);
			H2K_log("\ththread %d Now clear xe_set to 0x%08x\n", hthread, H2K_gp->xe_set);
		}
		if (!hthread_xe2 && (ret->ssr & SSR_XE2_BIT_MASK)) {  // new hthread with xe2 set
			XE2_SET_SET(hthread);
			H2K_log("\ththread %d Now set xe2_set to  0x%08x\n", hthread, H2K_gp->xe2_set);
		}
		if (hthread_xe2 && !(ret->ssr & SSR_XE2_BIT_MASK)) {
			XE2_SET_CLR(hthread);
			H2K_log("\ththread %d Now clear xe2_set to 0x%08x\n", hthread, H2K_gp->xe2_set);
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

	H2K_log("\ngetbest: start hthread %d\n", hthread);
	prio = H2K_ready_best_prio();
	if (prio >= MAX_PRIOS) {  // !H2K_ready_any_valid(), go to sleep
#ifdef CLUSTER_SCHED
		if (!H2K_gp->cluster_sched) {
			return NULL;
		}

		u32_t ssr = H2K_get_ssr();
		u32_t hthread_xe = ssr & SSR_XE_BIT_MASK;
		u32_t hthread_xe2 = ssr & SSR_XE2_BIT_MASK;

		if (hthread_xe) {
			XE_SET_CLR(hthread);
			ssr &= ~SSR_XE_BIT_MASK;
			H2K_log("getbest: hthread %d  xe_set 0x%08x\n", hthread, H2K_gp->xe_set);
		}
		if (hthread_xe2) {
			XE2_SET_CLR(hthread);
			ssr &= ~SSR_XE2_BIT_MASK;
			H2K_log("getbest: hthread %d  xe2_set 0x%08x\n", hthread, H2K_gp->xe2_set);
		}
		H2K_set_ssr(ssr);
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

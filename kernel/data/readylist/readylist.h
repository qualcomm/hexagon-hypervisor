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
# ifdef HAVE_HLX
static inline void H2K_update_coprocs(u32_t hthread, u32_t hthread_xe, u32_t hthread_xe2, u32_t hthread_xe3,u32_t head_xe, u32_t head_xe2, u32_t head_xe3) {//TODO: Does this need to be done for hlx, will it break other things
	xex_set_clr(hthread, (head_xe < hthread_xe), (head_xe2 < hthread_xe2), (head_xe3 < hthread_xe3));
	xex_set_set(hthread, (head_xe > hthread_xe), (head_xe2 > hthread_xe2), (head_xe3 > hthread_xe3));
	if (hthread_xe) {
		if (!head_xe) {
			H2K_log("hthread %d  update_coprocs: drop xe\n");
		}
	} else {
		if (head_xe) {
			H2K_log("hthread %d  update_coprocs: add xe\n");
		}
	}
	if (hthread_xe2) {
		if (!head_xe2) {
			H2K_log("hthread %d  update_coprocs: drop xe2\n");
		}
	} else {
		if (head_xe2) {
			H2K_log("hthread %d  update_coprocs: add xe2\n");
		}
	}
	if (hthread_xe3) {
		if (!head_xe3) {
			H2K_log("hthread %d  update_coprocs: drop xe3\n");
		}
	} else {
		if (head_xe3) {
			H2K_log("hthread %d  update_coprocs: add xe3\n");
		}
	}
}
# else
static inline void H2K_update_coprocs(u32_t hthread, u32_t hthread_xe, u32_t hthread_xe2, u32_t head_xe, u32_t head_xe2) {//TODO: Does this need to be done for hlx, will it break other things
	xex_set_clr(hthread, (head_xe < hthread_xe), (head_xe2 < hthread_xe2));
	xex_set_set(hthread, (head_xe > hthread_xe), (head_xe2 > hthread_xe2));
	if (hthread_xe) {
		if (!head_xe) {
			H2K_log("hthread %d  update_coprocs: drop xe\n");
		}
	} else {
		if (head_xe) {
			H2K_log("hthread %d  update_coprocs: add xe\n");
		}
	}
	if (hthread_xe2) {
		if (!head_xe2) {
			H2K_log("hthread %d  update_coprocs: drop xe2\n");
		}
	} else {
		if (head_xe2) {
			H2K_log("hthread %d  update_coprocs: add xe2\n");
		}
	}
}
# endif
#endif

/* Return head of ready list at prio */
static inline H2K_thread_context *H2K_ready_head(u32_t prio, u32_t hthread) {
	H2K_thread_context *head = H2K_gp->ready[prio];
	H2K_thread_context *ret = head;

#ifdef CLUSTER_SCHED
	if ((!H2K_gp->cluster_sched) || H2K_gp->coproc_max == -1) {
		return ret;
	}

	u32_t ssr = H2K_get_ssr();
	u32_t hthread_xe = ((ssr & SSR_XE_BIT_MASK) ? 1 : 0);
	u32_t hthread_xe2 = ((ssr & SSR_XE2_BIT_MASK) ? 1 : 0);
# ifdef HAVE_HLX
	u32_t hthread_xe3 = ((ssr & SSR_XE3_BIT_MASK) ? 1 : 0);//TODO: Change if there is an SSR 2 for HLX
	u32_t have = hthread_xe + hthread_xe2 + hthread_xe3; // # coprocs active on this thread
#else
	u32_t have = hthread_xe + hthread_xe2; // # coprocs active on this thread
# endif

	u32_t head_xe = ((ret->ssr_xe) ? 1 : 0);  // thread at head needs xe
	u32_t head_xe2 = ((ret->ssr_xe2) ? 1 : 0);  // thread at head needs xe2
# ifdef HAVE_HLX
	u32_t head_xe3 = ((ret->ssr_xe3) ? 1 : 0);  // thread at head needs xe3
	u32_t need = head_xe + head_xe2 + head_xe3; // total coprocs needed by thread at head
#else
	u32_t need = head_xe + head_xe2; // total coprocs needed by thread at head
# endif
	
	u32_t cluster = H2K_hthread_cluster(hthread);  // cluster of hthread
	u32_t victim;  // hthread to receive interrupt
	u32_t min_coprocs = -1;  // min # coprocs found on cluster with at least 1 waiting thread
	u32_t __attribute__((unused)) min_cluster = -1;  // cluster that has min coprocs and at least 1 waiting thread
	u32_t need_tmp = need;  // save # needed coprocs when walking ready list
	u32_t i;

# ifdef HAVE_HLX
	H2K_log("hthread %d  have %d  need %d  head_xe %d  head_xe2 %d  head_xe3 %d  counts %d %d %d %d \n\tcheck task 0x%08x\n", hthread, have, need, head_xe, head_xe2, head_xe3,H2K_gp->coproc_count[0], H2K_gp->coproc_count[1], H2K_gp->coproc_count[2], H2K_gp->coproc_count[3], ret);
#else
	H2K_log("hthread %d  have %d  need %d  head_xe %d  head_xe2 %d  counts %d %d %d %d \n\tcheck task 0x%08x\n", hthread, have, need, head_xe, head_xe2, H2K_gp->coproc_count[0], H2K_gp->coproc_count[1], H2K_gp->coproc_count[2], H2K_gp->coproc_count[3], ret);
# endif
	

	if (H2K_gp->coproc_count[cluster] + (need - have) <= H2K_gp->coproc_max) {  // within limit
# ifdef HAVE_HLX
		H2K_update_coprocs(hthread, hthread_xe, hthread_xe2, hthread_xe3, head_xe, head_xe2, head_xe3);
#else
		H2K_update_coprocs(hthread, hthread_xe, hthread_xe2, head_xe, head_xe2);
# endif
		H2K_log("hthread %d  change coprocs from %d to %d, new cluster count %d\n", hthread, have, need, H2K_gp->coproc_count[cluster]);
		return ret;  // pick up task
	}

	/* find the least-loaded cluster that has thread(s) in wait mode */
	if (H2K_gp->wait_mask) {  // only check if there are waiting threads
		for (i = 0; i < H2K_gp->cluster_clusters; i++) {
			if (H2K_gp->wait_mask & H2K_gp->cluster_mask[i]) {  // threads in wait mode
				if (H2K_gp->coproc_count[i] < min_coprocs) {  // new min
					min_coprocs = H2K_gp->coproc_count[i];
					min_cluster = i;
					victim = H2K_gp->wait_mask & H2K_gp->cluster_mask[i];
					H2K_log("hthread %d  min cluster %d, min coprocs %d, victims 0x%08x\n", hthread, min_cluster, min_coprocs, victim);
				}
			}
		}
	} else {  // no eligible cluster found, so pick up task on this hthread
# ifdef HAVE_HLX
		H2K_update_coprocs(hthread, hthread_xe, hthread_xe2, hthread_xe3, head_xe, head_xe2, head_xe3);
#else
		H2K_update_coprocs(hthread, hthread_xe, hthread_xe2, head_xe, head_xe2);
# endif
		H2K_log("hthread %d  clusters full; change coprocs from %d to %d, new cluster count %d\n", hthread, have, need, H2K_gp->coproc_count[cluster]);
		return ret;
	}

	/* Found a min cluster with threads in wait mode; try to hand off task */
	do {
		if (min_coprocs + need_tmp <= H2K_gp->coproc_max) {  // other cluster within limit
			victim = Q6_R_ct0_R(victim);
			H2K_log("hthread %d Signal thread %d\n", hthread, victim);
			iassignw(CLUSTER_RESCHED_INT, ~(0x1 << victim));  // steer the interrupt
			cluster_resched_int();    // get victim hthread to pick up what we skipped
			/* This hthread is going to sleep, so no longer using xe/xe2/xe3 */
			ssr &= ~SSR_XE_BIT_MASK;
			ssr &= ~SSR_XE2_BIT_MASK;
# ifdef HAVE_HLX
			ssr &= ~SSR_XE3_BIT_MASK;
# endif

			H2K_set_ssr(ssr);

# ifdef HAVE_HLX
			xex_set_clr(hthread, hthread_xe, hthread_xe2, hthread_xe3);
			H2K_log("hthread %d  sleeping 1, hthread_xe %d  hthread_xe2 %d  hthread_xe2 %d  new cluster count %d\n", hthread, hthread_xe, hthread_xe2,hthread_xe3, H2K_gp->coproc_count[cluster]);
# else
			xex_set_clr(hthread, hthread_xe, hthread_xe2);
			H2K_log("hthread %d  sleeping 1, hthread_xe %d  hthread_xe2 %d  new cluster count %d\n", hthread, hthread_xe, hthread_xe2, H2K_gp->coproc_count[cluster]);
# endif
			return NULL;
		}

		/* else walk the ready list until we find a task that fits on min_cluster */
		do 
		{
			ret = (H2K_thread_context *)H2K_ring_next(head, ret);
		}
# ifdef HAVE_HLX 
		while (ret != NULL && (need_tmp = (((ret->ssr_xe) ? 1 : 0) + ((ret->ssr_xe2) ? 1 : 0) + ((ret->ssr_xe3) ? 1 : 0)) + min_coprocs) > H2K_gp->coproc_max);
# else
		while (ret != NULL && (need_tmp = (((ret->ssr_xe) ? 1 : 0) + ((ret->ssr_xe2) ? 1 : 0)) + min_coprocs) > H2K_gp->coproc_max);
# endif
		
		H2K_log("hthread %d try new task 0x%08x\n", ret);
	} while (ret != NULL);
	
	/* Didn't find another task to schedule, so pick up the head task even though this unbalances the coprocs */
# ifdef HAVE_HLX
	H2K_update_coprocs(hthread, hthread_xe, hthread_xe2, hthread_xe3, head_xe, head_xe2, head_xe3);
#else
	H2K_update_coprocs(hthread, hthread_xe, hthread_xe2, head_xe, head_xe2);
# endif
	H2K_log("hthread %d  increase coprocs from %d to %d, new cluster count %d\n", hthread, have, need, H2K_gp->coproc_count[cluster]);
 
#endif

	return head;
}

/* Remove and return the best thread */
/* me == thread being switched out */
static inline H2K_thread_context *H2K_ready_getbest(u32_t hthread)
{
	H2K_thread_context *ret;
	u32_t prio;

	H2K_log("hthread %d  getbest\n", hthread);
	prio = H2K_ready_best_prio();
	if (prio >= MAX_PRIOS) {  // !H2K_ready_any_valid(), go to sleep
#ifdef CLUSTER_SCHED
		if (!H2K_gp->cluster_sched) {
			return NULL;
		}

		u32_t ssr = H2K_get_ssr();
		u32_t hthread_xe = ((ssr & SSR_XE_BIT_MASK) ? 1 : 0);
		u32_t hthread_xe2 = ((ssr & SSR_XE2_BIT_MASK) ? 1 : 0);
# ifdef HAVE_HLX 
		u32_t hthread_xe3 = ((ssr & SSR_XE3_BIT_MASK) ? 1 : 0);//TODO: Change if there is an SSR 2 for HLX
# endif
		/* This hthread is goint to sleep, so no longer using xe/xe2 */
		ssr &= ~SSR_XE_BIT_MASK;
		ssr &= ~SSR_XE2_BIT_MASK;
# ifdef HAVE_HLX
		ssr &= ~SSR_XE3_BIT_MASK;
# endif
		H2K_set_ssr(ssr);
# ifdef HAVE_HLX
		xex_set_clr(hthread, hthread_xe, hthread_xe2, hthread_xe3);
		H2K_log("hthread %d  sleeping 2, hthread_xe %d  hthread_xe2 %d  hthread_xe2 %d  new cluster count %d\n", hthread, hthread_xe, hthread_xe2, hthread_xe2, H2K_gp->coproc_count[H2K_hthread_cluster(hthread)]);
# else
		xex_set_clr(hthread, hthread_xe, hthread_xe2);
		H2K_log("hthread %d  sleeping 2, hthread_xe %d  hthread_xe2 %d  new cluster count %d\n", hthread, hthread_xe, hthread_xe2, H2K_gp->coproc_count[H2K_hthread_cluster(hthread)]);
# endif
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

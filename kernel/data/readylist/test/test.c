/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <hw.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <cfg_table.h>

/*
 * Strategy: 
 * Place the ready list in several states, manipulate it using the appropriate
 * functions, and check that the values are expected 
 */

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context a,b,c;
#define CURRENT_HTHREAD 0
#define GARBAGE_VALUE 0x5a5a5a5a5a5a5a5aULL

static inline void poison_ready_valids(void)
{
	for (u32_t i = 0; i < MAX_PRIOS/64; i++) {
		H2K_gp->ready_valids[i] = GARBAGE_VALUE;
	}
}

static inline void poison_ready(void)
{
	for (u32_t i = 0; i < MAX_PRIOS; i++) {
		H2K_gp->ready[i] = (H2K_thread_context *)GARBAGE_VALUE;
	}
}

/* These functions create a real version of the inlines in readylist.h */
u32_t H2K_ready_best_prio_TB()
{
	return H2K_ready_best_prio();
}

u32_t H2K_ready_any_valid_TB()
{
	return H2K_ready_any_valid();
}

u32_t H2K_ready_prio_valid_TB(u32_t prio)
{
	return H2K_ready_prio_valid(prio);
}

void H2K_ready_set_prio_TB(u32_t prio)
{
	H2K_ready_set_prio(prio);
}

void H2K_ready_clear_prio_TB(u32_t prio)
{
	H2K_ready_clear_prio(prio);
}

void H2K_ready_append_TB(H2K_thread_context *thread)
{
	H2K_ready_append(thread);
}

void H2K_ready_insert_TB(H2K_thread_context *thread)
{
	H2K_ready_insert(thread);
}

void H2K_ready_remove_TB(H2K_thread_context *thread)
{
	H2K_ready_remove(thread);
}

H2K_thread_context *H2K_ready_getbest_TB()
{
	return H2K_ready_getbest(CURRENT_HTHREAD);
}

#if CLUSTER_SCHED
void H2K_ready_REG_SSR_XE_CLEAR_TB()
{
	H2K_set_ssr(H2K_get_ssr() & ~SSR_XE_BIT_MASK);
}

void H2K_ready_REG_SSR_XE2_CLEAR_TB()
{
	H2K_set_ssr(H2K_get_ssr() & ~SSR_XE2_BIT_MASK);
}

void H2K_ready_REG_SSR_XE_SET_TB()
{
	H2K_set_ssr(H2K_get_ssr() | SSR_XE_BIT_MASK);
}

void H2K_ready_REG_SSR_XE2_SET_TB()
{
	H2K_set_ssr(H2K_get_ssr() | SSR_XE2_BIT_MASK);
}

void H2K_ready_REG_CCR_XE3_CLEAR_TB()
{
	H2K_set_ccr(H2K_get_ccr() & ~CCR_XE3_BIT_MASK);
}

void H2K_ready_REG_CCR_XE3_SET_TB()
{
	H2K_set_ccr(H2K_get_ccr() | CCR_XE3_BIT_MASK);
}


u32_t H2K_ready_create_hthreadmask_TB(u32_t hthreadstartidx, u32_t hthreadstopidx)
{
	u32_t hthreadmask = 0;
	for (u32_t hthread = hthreadstartidx; hthread <= hthreadstopidx; hthread++) {
	    hthreadmask |= (0x1 << hthread);
	}
	return hthreadmask;
}

void H2K_ready_THREAD_XE_CLEAR_TB(H2K_thread_context *thread)
{
	thread->ssr &= ~SSR_XE_BIT_MASK;
}

void H2K_ready_THREAD_XE_SET_TB(H2K_thread_context *thread)
{
	thread->ssr |= SSR_XE_BIT_MASK;
}

void H2K_ready_THREAD_XE2_CLEAR_TB(H2K_thread_context *thread)
{
	thread->ssr &= ~SSR_XE2_BIT_MASK;
}

void H2K_ready_THREAD_XE2_SET_TB(H2K_thread_context *thread)
{
	thread->ssr |= SSR_XE2_BIT_MASK;
}

void H2K_ready_THREAD_XE3_CLEAR_TB(H2K_thread_context *thread)
{
	thread->ccr &= ~CCR_XE3_BIT_MASK;
}

void H2K_ready_THREAD_XE3_SET_TB(H2K_thread_context *thread)
{
	thread->ccr |= CCR_XE3_BIT_MASK;
}

#endif

int main() 
{
__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	
	poison_ready_valids();
	poison_ready();
	H2K_readylist_init();
	if (H2K_ready_any_valid_TB()) FAIL("readylist_init failed to set valids");
	for (u32_t i = 0; i < MAX_PRIOS; i++) {
    	if (H2K_gp->ready[i] != NULL) FAIL("readylist_init failed to clear array");
	}

	b.prio = a.prio = 2;
	c.prio = 4;
	H2K_ready_append_TB(&a);
	if (H2K_gp->ready[2] != &a) FAIL("ready_append failed (0) ");
	if (!H2K_ready_prio_valid_TB(2)) FAIL("ready_append failed (1) ");
	if (H2K_gp->ready[2]->next != &a) FAIL("ready_append failed (2) ");
	if (H2K_gp->ready[2]->prev != &a) FAIL("ready_append failed (3) ");
	if (H2K_ready_best_prio_TB() != 2) FAIL("Ready best prio (4) ");

	H2K_ready_append_TB(&b);
	if (H2K_gp->ready[2] != &a) FAIL("ready_append failed (5)");
	if (!H2K_ready_prio_valid_TB(2)) FAIL("ready_append failed (6)");
	if (H2K_gp->ready[2]->next != &b) FAIL("ready_append failed (7)");
	if (H2K_gp->ready[2]->prev != &b) FAIL("ready_append failed (8)");
	if (H2K_gp->ready[2]->next->next != &a) FAIL("ready_append failed (9)");
	if (H2K_gp->ready[2]->prev->prev != &a) FAIL("ready_append failed (10)");
	if (H2K_ready_best_prio_TB() != 2) FAIL("Ready best prio");

	H2K_readylist_init();
	H2K_ready_insert_TB(&a);
	if (H2K_gp->ready[2] != &a) FAIL("ready_insert failed");
	if (!H2K_ready_prio_valid_TB(2)) FAIL("ready_insert failed");
	if (H2K_gp->ready[2]->next != &a) FAIL("ready_insert failed");
	if (H2K_gp->ready[2]->prev != &a) FAIL("ready_insert failed");
	if (H2K_ready_best_prio_TB() != 2) FAIL("Ready best prio");

	H2K_ready_insert_TB(&b);
	if (H2K_gp->ready[2] != &b) FAIL("ready_insert failed");
	if (!H2K_ready_prio_valid_TB(2)) FAIL("ready_insert failed");
	if (H2K_gp->ready[2]->next != &a) FAIL("ready_insert failed");
	if (H2K_gp->ready[2]->prev != &a) FAIL("ready_insert failed");
	if (H2K_gp->ready[2]->next->next != &b) FAIL("ready_insert failed");
	if (H2K_gp->ready[2]->prev->prev != &b) FAIL("ready_insert failed");
	if (H2K_ready_best_prio_TB() != 2) FAIL("Ready best prio");

	H2K_ready_remove_TB(&a);
	if (H2K_gp->ready[2] != &b) FAIL("ready_remove failed");
	if (!H2K_ready_prio_valid_TB(2)) FAIL("ready_remove failed");
	if (H2K_gp->ready[2]->next != &b) FAIL("ready_remove failed");
	if (H2K_gp->ready[2]->prev != &b) FAIL("ready_remove failed");
	if (H2K_ready_best_prio_TB() != 2) FAIL("Ready best prio");

	H2K_ready_remove_TB(&b);
	if (H2K_gp->ready[2] != NULL) FAIL("ready_remove failed");
	if (H2K_ready_any_valid_TB()) FAIL("ready_remove failed");

	H2K_ready_insert_TB(&a);
	H2K_ready_insert_TB(&b);
	H2K_ready_insert_TB(&c);

	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed (b) ");
	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed (a) ");
	if (H2K_ready_getbest_TB() != &c) FAIL("ready_best_prio failed (c) ");
	if (H2K_ready_getbest_TB() != NULL) FAIL("ready_best_prio failed (empty) ");

# if CLUSTER_SCHED
	
	u32_t hthreadmask = H2K_cfg_table(CFG_TABLE_HTHREADS_MASK);
	u32_t hthreads = (u32_t)Q6_R_popcount_P(hthreadmask);
	/* FIXME: need a cfg_table entry for this */
	H2K_gp->cluster_clusters = (hthreads > 8 ? 4 : 2); // hack
	H2K_gp->cluster_hthreads = hthreads / H2K_gp->cluster_clusters;
	for (u32_t i = 0; i < CLUSTER_MAX_CLUSTERS; i++) {
		H2K_gp->cluster_mask[i] = BITS_MASK(H2K_gp->cluster_hthreads, H2K_gp->cluster_hthreads * i);
	}

	H2K_gp->cluster_sched = 1;
	u32_t cluster1 = H2K_hthread_cluster(CURRENT_HTHREAD);
	u32_t cluster2 = (cluster1 + 1) % H2K_gp->cluster_clusters;

#if ARCHV > 65
	H2K_gp->coproc_contexts = H2K_cfg_table(CFG_TABLE_COPROC_CONTEXTS);
#else
	H2K_gp->coproc_contexts = EXT_HVX_CONTEXTS;
#endif

#if ARCHV >= 81
	u32_t val = H2K_cfg_table(CFG_TABLE_HMX_INT8_RATE);
	for (u32_t i = 0; i < sizeof(val); i++) {
		H2K_gp->hmx_units += ((val & BITS_MASK(8, i * 8)) != 0);  // byte not 0?
	}
#else
	H2K_gp->hmx_units = (H2K_cfg_table(CFG_TABLE_HMX_INT8_RATE) != 0);  // exists?
#endif
	H2K_gp->coproc_max = ((H2K_gp->coproc_contexts + H2K_gp->hmx_units) / H2K_gp->cluster_clusters) + (((H2K_gp->coproc_contexts + H2K_gp->hmx_units) % H2K_gp->cluster_clusters) != 0);
	H2K_gp->coproc_max = (H2K_gp->coproc_max < CLUSTER_SCHED_MIN_COPROCS ? CLUSTER_SCHED_MIN_COPROCS : H2K_gp->coproc_max);

	// Base case: cluster scheduling disabled -> early out, head returned regardless
	// of coproc budget. Even with the cluster "full" and the thread needing a coproc,
	// getbest must return it (the !cluster_sched / coproc_max == -1 guard).
	H2K_gp->coproc_count[cluster1] = 0; // cluster full
	H2K_ready_THREAD_XE_SET_TB(&a);                      // a needs a coproc
	u32_t saved_max = H2K_gp->coproc_max;

	// sub-case 1: cluster_sched == 0
	H2K_gp->cluster_sched = 0;
	H2K_ready_insert_TB(&a);
	if (H2K_ready_getbest_TB() != &a) FAIL("early-out: !cluster_sched did not return head");
	H2K_gp->cluster_sched = 1;

	// sub-case 2: coproc_max == -1 sentinel
	H2K_gp->coproc_max = (u32_t)-1;
	H2K_ready_insert_TB(&a);
	if (H2K_ready_getbest_TB() != &a) FAIL("early-out: coproc_max == -1 did not return head");
	H2K_gp->coproc_max = saved_max;
	H2K_ready_THREAD_XE_CLEAR_TB(&a);


	// First correctness check: HVX (xe) and HLX (xe3) share ONE coproc slot.
	// Picking up a thread that needs xe charges the cluster 1 slot; setting xe3
	// on top of xe must still charge only 1 (not 2). coproc_count is reset before
	// each pickup so we measure the per-pickup delta in isolation.
	H2K_ready_REG_SSR_XE_CLEAR_TB(); // running hthread holds no coproc
	H2K_ready_REG_SSR_XE2_CLEAR_TB();
	H2K_ready_REG_CCR_XE3_CLEAR_TB();

	// xe only -> picking up a charges 1 slot
	H2K_gp->coproc_count[cluster1] = 0;
	H2K_ready_THREAD_XE_SET_TB(&a);
	H2K_ready_insert_TB(&a);
	if (H2K_ready_getbest_TB() != &a) FAIL("getbest xe: a not picked");
	if (H2K_gp->coproc_count[cluster1] != 1) FAIL("getbest xe: count != 1");

	// xe AND xe3 -> still only 1 slot (shared)
	H2K_gp->coproc_count[cluster1] = 0;
	H2K_ready_THREAD_XE3_SET_TB(&a);
	H2K_ready_insert_TB(&a);
	if (H2K_ready_getbest_TB() != &a) FAIL("getbest xe|xe3: a not picked");
	if (H2K_gp->coproc_count[cluster1] != 1) FAIL("getbest xe|xe3: count != 1");

	// xe AND xe3 AND xe2 -> picking up 2 slots
	H2K_gp->coproc_count[cluster1] = 0;
	H2K_ready_THREAD_XE2_SET_TB(&a);
	H2K_ready_insert_TB(&a);
	if (H2K_ready_getbest_TB() != &a) FAIL("getbest (xe|xe3) & xe2: a not picked");
	if (H2K_gp->coproc_count[cluster1] != 2) FAIL("getbest (xe|xe3) & xe2: count != 2");

	H2K_ready_THREAD_XE_CLEAR_TB(&a);
	H2K_ready_THREAD_XE2_CLEAR_TB(&a);
	H2K_ready_THREAD_XE3_CLEAR_TB(&a);


	// Case: cluster is at full coproc capacity, NO idle hthread anywhere to hand off to.
	// running hthread holds nothing, one thread ready that needs a coproc.
	// Expected: scheduler can't delegate, so it runs the task locally even though
	// that pushes the cluster over coproc_max.
	H2K_gp->coproc_count[cluster1] = H2K_gp->coproc_max; // cluster full
	H2K_gp->wait_mask = 0;                               // no idle hthread to hand off to
	H2K_ready_THREAD_XE_SET_TB(&a);  // a needs a coproc
	H2K_ready_insert_TB(&a);

	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed (full-no-waiters: a not picked) ");
	if (H2K_gp->coproc_count[cluster1] != H2K_gp->coproc_max + 1) FAIL("full-no-waiters: count not oversubscribed");


	// Case: cluster is at full coproc capacity, idle threads in other cluster,
	// running hthread holds nothing, two threads ready:
	// td(a) needs a coproc, td(b) doesn't.
	// Expected: b is picked (fits), then nothing.
	H2K_gp->coproc_count[cluster1] = H2K_gp->coproc_max; // cluster full
	H2K_gp->coproc_count[cluster2] = 0;                     // another cluster has room
	H2K_gp->wait_mask = H2K_gp->cluster_mask[cluster2];     // an idle hthread exists in that cluster
	H2K_ready_THREAD_XE_SET_TB(&a); // set xe for td(i)
	H2K_ready_insert_TB(&a); // insert td(i) to list
	H2K_ready_THREAD_XE_CLEAR_TB(&b); // clr xe for td(ii)
	H2K_ready_insert_TB(&b); // insert td(ii) to list
	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed");
	if (H2K_ready_getbest_TB() != NULL) FAIL("ready_best_prio failed");



	// Case: cluster has one free coproc slot, running hthread holds nothing,
	// two equal-priority threads ready: td(b) needs no coproc, td(a) needs one.
	// Expected: both are picked -- b fits trivially, then a fits in the last slot.
	H2K_gp->coproc_count[cluster1] = H2K_gp->coproc_max - 1; // one slot free
	H2K_ready_THREAD_XE_SET_TB(&a); // set xe for td(i)
	H2K_ready_THREAD_XE_CLEAR_TB(&b); // clr xe for td(ii)
	H2K_ready_insert_TB(&a); // insert td(i) to list
	H2K_ready_insert_TB(&b); // insert td(ii) to list
	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed");
	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed");



	// Case: running hthread holds a coproc (have=1), cluster at full capacity.
	// td(a) needs a coproc, td(b) doesn't (both same prio).
	// Expected: b picked (need-have = 0-1 = -1, fits), then a picked (need-have = 1-1 = 0, fits).
	H2K_ready_REG_SSR_XE_SET_TB(); // running hthread holds a coproc (have=1)
	H2K_gp->coproc_count[cluster1] = H2K_gp->coproc_max; // cluster full
	H2K_ready_THREAD_XE_SET_TB(&a); // set xe for td(i)
	H2K_ready_THREAD_XE_CLEAR_TB(&b); // clr xe for td(ii)
	H2K_ready_insert_TB(&a); // insert td(i) to list
	H2K_ready_insert_TB(&b); // insert td(ii) to list
	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed");
	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed");
	H2K_ready_REG_SSR_XE_CLEAR_TB();



	// Case: cluster full, an idle hthread exists but its cluster is also full, and
	// the single ready thread needs a coproc. Handoff can't fit it elsewhere and the
	// ring walk finds no other thread.
	// Expect a picked and the cluster oversubscribed.
	for (u32_t i = 0; i < CLUSTER_MAX_CLUSTERS; i++) {
		H2K_gp->coproc_count[i] = H2K_gp->coproc_max; // every cluster full
	}
	H2K_gp->wait_mask = H2K_gp->cluster_mask[cluster2];  // idle hthread there, but no room
	H2K_ready_THREAD_XE_SET_TB(&a); // a needs a coproc
	H2K_ready_insert_TB(&a);
	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed");
	if (H2K_gp->coproc_count[cluster1] != H2K_gp->coproc_max + 1) FAIL("count not oversubscribed");


	// Case: cluster full, other cluster has ONE free slot. Head thread (a) needs 2
	// coprocs so it doesn't fit there, but a deeper thread (b) needs 0 and does. The
	// ring walk skips a, finds b fits min_cluster, hands off and sleeps (return NULL).
	H2K_gp->coproc_count[cluster1] = H2K_gp->coproc_max;     // this cluster full
	H2K_gp->coproc_count[cluster2] = H2K_gp->coproc_max - 1; // other cluster: one slot free
	H2K_gp->wait_mask = H2K_gp->cluster_mask[cluster2];      // idle hthread there
	H2K_ready_THREAD_XE_SET_TB(&a);  // a needs 2 coprocs (xe + xe2)
	H2K_ready_THREAD_XE2_SET_TB(&a);  // a needs 2 coprocs (xe + xe2)
	H2K_ready_THREAD_XE_CLEAR_TB(&b); // b needs none
	H2K_ready_THREAD_XE2_CLEAR_TB(&b);  // b needs none
	H2K_ready_insert_TB(&b);          // b deeper
	H2K_ready_insert_TB(&a);          // a at head
	if (H2K_ready_getbest_TB() == &b) FAIL("ready_best_prio failed");
	if (H2K_ready_getbest_TB() == &b) FAIL("ready_best_prio failed");
	H2K_ready_remove_TB(&a);          // handoff left a/b in the list; clean up
	H2K_ready_remove_TB(&b);


	// Case: All prio up to maxprio reset via init, and register sse xe set,
	// and single td inserted with its prio re-reset,
	// should then not find/get any best td as no td valid ready (as all prio up to maxprio reset).
	H2K_readylist_init(); // reset all prio upto maxprio
	H2K_ready_REG_SSR_XE_SET_TB(); // set xe for ssr reg
	H2K_ready_insert_TB(&c); // insert td to list
	H2K_ready_clear_prio(c.prio); // reset td prio

	if (H2K_ready_getbest_TB() != NULL) FAIL("ready_best_prio failed (maxprio) ");
#endif


	puts("TEST PASSED\n");
	return 0;
}

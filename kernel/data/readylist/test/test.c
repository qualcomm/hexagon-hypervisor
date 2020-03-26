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
	return H2K_ready_getbest(0);
}

#ifdef CLUSTER_SCHED
void H2K_ready_REG_SSR_XE_CLEAR_TB()
{
	H2K_set_ssr(H2K_get_ssr() & ~SSR_XE_BIT_MASK);
}

void H2K_ready_REG_SSR_XE_SET_TB()
{
	H2K_set_ssr(H2K_get_ssr() | SSR_XE_BIT_MASK);
}

u32_t H2K_ready_create_hthreadmask_TB(u32_t hthreadstartidx, u32_t hthreadstopidx)
{
	u32_t hthreadmask = 0;
	for (u32_t hthread = hthreadstartidx; hthread <= hthreadstopidx; hthread++) {
	    hthreadmask |= (0x1 << hthread);
	}
	return hthreadmask;
}

void H2K_ready_CLUSTER_XE_CLEAR_TB(u32_t hthreadmask)
{
	u32_t cluster = H2K_hthread_cluster(0);
	H2K_gp->xe_set[cluster] &= ~hthreadmask;
}

void H2K_ready_CLUSTER_XE_SET_TB(u32_t hthreadmask)
{
	u32_t cluster = H2K_hthread_cluster(0);
	H2K_gp->xe_set[cluster] |= hthreadmask;
}

void H2K_ready_THREAD_XE_CLEAR_TB(H2K_thread_context *thread)
{
	thread->ssr &= ~SSR_XE_BIT_MASK;
}

void H2K_ready_THREAD_XE_SET_TB(H2K_thread_context *thread)
{
	thread->ssr |= SSR_XE_BIT_MASK;
}
#endif

int main() 
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_gp->ready_valids[0] = 0xfeca1feddeadbeefULL;
	H2K_gp->ready_valids[1] = 0xfa7510b5ca5cadedULL;
	H2K_gp->ready_valids[2] = 0x5a5a5a5a5a5a5a5aULL;
	H2K_gp->ready_valids[3] = 0xf0f0f0f0f0f0f0f0ULL;
	H2K_gp->ready[0] = &a;
	H2K_readylist_init();
	if (H2K_ready_any_valid_TB()) FAIL("readylist_init failed to set valids");
	if (H2K_gp->ready[0] != 0) FAIL("readylist_init failed to clear array");
	if (H2K_ready_best_prio_TB() <= MAX_PRIO) FAIL("cleared readylist best prio <= MAX_PRIO");

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

#ifdef CLUSTER_SCHED
	u32_t hthreadmask = 0;
	H2K_gp->cluster_sched = 1;
	H2K_gp->cluster_hthreads = (u32_t)(Q6_R_popcount_P(H2K_cfg_table(CFG_TABLE_HTHREADS_MASK)) / 2);
	H2K_gp->cluster_mask[0] = (u32_t)(0xffff >> (16 - H2K_gp->cluster_hthreads));
	H2K_gp->cluster_mask[1] = (u32_t)((0xffff >> (16 - H2K_gp->cluster_hthreads)) << H2K_gp->cluster_hthreads);

	H2K_ready_REG_SSR_XE_CLEAR_TB();
	hthreadmask = H2K_ready_create_hthreadmask_TB(0, MAX_HVX_PER_CLUSTER-1);
	H2K_ready_CLUSTER_XE_SET_TB(hthreadmask);
	H2K_ready_THREAD_XE_SET_TB(&a);
	H2K_ready_THREAD_XE_CLEAR_TB(&b);
	H2K_ready_insert_TB(&a);
	H2K_ready_insert_TB(&b);

	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed (b1) ");
	if (H2K_ready_getbest_TB() != NULL) FAIL("ready_best_prio failed (a1) ");

	H2K_ready_REG_SSR_XE_CLEAR_TB();
	hthreadmask = H2K_ready_create_hthreadmask_TB(0, 0);
	H2K_ready_CLUSTER_XE_SET_TB(hthreadmask);
	hthreadmask = H2K_ready_create_hthreadmask_TB(1, MAX_HVX_PER_CLUSTER-1);
	H2K_ready_CLUSTER_XE_CLEAR_TB(hthreadmask);
	H2K_ready_THREAD_XE_CLEAR_TB(&a);
	H2K_ready_THREAD_XE_SET_TB(&b);
	H2K_ready_insert_TB(&a);
	H2K_ready_insert_TB(&b);

	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed (b2) ");
	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed (a2) ");

	H2K_ready_REG_SSR_XE_SET_TB();
	hthreadmask = H2K_ready_create_hthreadmask_TB(0, 0);
	H2K_ready_CLUSTER_XE_SET_TB(hthreadmask);
	hthreadmask = H2K_ready_create_hthreadmask_TB(1, MAX_HVX_PER_CLUSTER-1);
	H2K_ready_CLUSTER_XE_CLEAR_TB(hthreadmask);
	H2K_ready_THREAD_XE_SET_TB(&a);
	H2K_ready_THREAD_XE_CLEAR_TB(&b);
	H2K_ready_insert_TB(&a);
	H2K_ready_insert_TB(&b);

	if (H2K_ready_getbest_TB() != &b) FAIL("ready_best_prio failed (b3) ");
	if (H2K_ready_getbest_TB() != &a) FAIL("ready_best_prio failed (a3) ");

	H2K_readylist_init(); // init set prio at maxprio
	H2K_ready_REG_SSR_XE_SET_TB(); // set ssr xe bits
	H2K_ready_insert_TB(&c); // insert thread to list
	H2K_ready_clear_prio(c.prio); // reset prio to max

	if (H2K_ready_getbest_TB() != NULL) FAIL("ready_best_prio failed (maxprio) ");
#endif

	puts("TEST PASSED\n");
	return 0;
}

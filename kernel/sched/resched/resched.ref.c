/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <hw.h>
#include <runlist.h>
#include <readylist.h>
#include <dosched.h>
#include <lowprio.h>
#include <resched.h>
#include <globals.h>
#include <log.h>
#include <thread.h>
#include <asid.h>
#include <timer.h>
#include <stop.h>

/* Reap a context whose vmblock is in the exiting state.  Caller holds BKL.
 * Used when a CLUSTER_RESCHED_INT (or RESCHED_INT) lands on a HW thread that
 * was running a context belonging to a vmblock whose main thread has exited.
 * Mirrors the per-context cleanup in H2K_thread_stop. */
static inline void self_reap_locked(H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = me->vmblock;
	H2K_runlist_remove(me);
	H2K_timer_cancel_withlock(me);
	H2K_asid_table_dec(me->ssr_asid);
	H2K_thread_context_clear(me);  /* preserves vmblock_id */
	me->next = vmblock->free_threads;
	vmblock->free_threads = me;
	vmblock->num_cpus--;
	H2K_vmblock_finalize_if_done_locked(vmblock);
}

static inline void resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum) {
	if (me != NULL && me->vmblock != NULL && me->vmblock->exiting) {
		self_reap_locked(me);
		H2K_dosched(NULL, hwtnum);
		/* unreachable */
	}
	if (me != NULL) {
		H2K_runlist_remove(me);
		H2K_ready_append(me);
	} else {
		/* Interrupted WAIT mode */
		H2K_gp->wait_mask = Q6_R_clrbit_RR(H2K_gp->wait_mask,hwtnum);
	}
	H2K_dosched(me, hwtnum);
}

void H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&H2K_bkl);
	resched(unused, me, hwtnum);
}

void H2K_resched_cluster(u32_t unused, H2K_thread_context *me, u32_t hwtnum)
{
	H2K_log("HW thread %d got CLUSTER_RESCHED_INT\n", hwtnum);
	ciad(CLUSTER_RESCHED_INT_INTMASK);
	BKL_LOCK(&H2K_bkl);
	iassignw(CLUSTER_RESCHED_INT, -1);  // disable for all hw threads
	resched(unused, me, hwtnum);
}

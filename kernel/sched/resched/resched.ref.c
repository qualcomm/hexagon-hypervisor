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
#include <timer.h>
#include <stop.h>

static inline void resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum) {
	if (likely(me != NULL)) {
		H2K_runlist_remove(me);
		if (unlikely(me->vmblock != NULL && me->vmblock->exiting)) {
			/* VM is tearing down (a peer called H2K_vm_stop and IPI'd us
			 * via CLUSTER_RESCHED_INT): reap self instead of requeuing,
			 * and hand dosched a NULL "previous" so our context isn't saved. */
			H2K_timer_cancel_withlock(me);
			H2K_free_context_locked(me->vmblock, me);
			H2K_vmblock_finalize_if_done_locked(me->vmblock);
			me = NULL;
		} else {
			H2K_ready_append(me);
		}
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

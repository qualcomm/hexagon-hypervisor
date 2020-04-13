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

static inline void resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum) {
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

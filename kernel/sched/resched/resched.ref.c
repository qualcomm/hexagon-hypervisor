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

void H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&H2K_bkl);
	if (me != NULL) {
		H2K_runlist_remove(me);
		H2K_ready_append(me);
	} else {
		/* Interrupted WAIT mode */
		H2K_wait_mask = Q6_R_clrbit_RR(H2K_wait_mask,hwtnum);
	}
	H2K_dosched(me, hwtnum);
}


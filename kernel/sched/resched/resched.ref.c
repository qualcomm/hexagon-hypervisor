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

void BLASTK_resched(u32_t unused, BLASTK_thread_context *me, u32_t hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&BLASTK_bkl);
	if (me != NULL) {
		BLASTK_runlist_remove(me);
		BLASTK_ready_append(me);
	} else {
		/* Interrupted WAIT mode */
		BLASTK_wait_mask ^= 1<<hwtnum;
	}
	BLASTK_dosched(me, hwtnum);
}


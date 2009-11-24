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

void BLASTK_reshcedule_from_wait(u32_t hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&BLASTK_bkl);
	BLASTK_wait_mask ^= 1<<hwtnum;
	BLASTK_dosched(NULL,hwtnum);
}

void BLASTK_reschedule_from_lowprio(u32_t unused, BLASTK_thread_context *me, u32_t hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&BLASTK_bkl);
	BLASTK_runlist_remove(me);
	BLASTK_ready_append(me);
	BLASTK_dosched(me, hwtnum);
}


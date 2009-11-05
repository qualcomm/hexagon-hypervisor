/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

void BLASTK_reshcedule_from_wait(int hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&BLASTK_bkl);
	BLASTK_wait_mask ^= 1<<hwtnum;
	BLASTK_dosched(NULL,hwtnum);
}

void BLASTK_reschedule_from_lowprio(int unused, BLASTK_thread_context *me, int hwtnum)
{
	ciad(RESCHED_INT_INTMASK);
	BKL_LOCK(&BLASTK_bkl);
	runlist_remove(me);
	ready_append(me);
	BLASTK_dosched(me, hwtnum);
}


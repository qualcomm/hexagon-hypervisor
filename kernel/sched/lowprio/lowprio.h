/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef LOWPRIO_H
#define LOWPRIO_H 1

/* Notify a low priority thread that it is the new lowest priority if necessary */
/* BLASTK_priomask should be non-zero */
static inline void lowprio_notify()
{
	u32_t prio;
	u32_t hthread;
	BLASTK_thread_context *tmp;
	prio = get_worst_running_prio();
	tmp = BLASTK_runlist[prio];
	hthread = BLASTK_runlist[prio]->hthread;
	BLASTK_priomask |= 1<<hthread;
	BLASTK_change_imask(hthread,0);
}

/* Notify the current lowprio thread that he is no longer the worst */
/* Note that if threads are waiting, this should not be called, you are
 * going to get into trouble if you raise the priority of a waiting thread */
static inline void raise_lowprio()
{
	u32_t mask = BLASTK_priomask;
	if (BLASTK_wait_mask) return; // just a sanity check... should be an error
	BLASTK_priomask = 0;
	BLASTK_change_imask(Q6_R_ct0_R(mask),-1);
}

#endif


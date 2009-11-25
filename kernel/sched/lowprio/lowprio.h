/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef LOWPRIO_H
#define LOWPRIO_H 1

#include <runlist.h>
#include <q6protos.h>
#include <hw.h>

extern u32_t BLASTK_priomask IN_SECTION(".data.sched.lowprio");
extern u32_t BLASTK_wait_mask IN_SECTION(".data.sched.lowprio");

/* Notify a low priority thread that it is the new lowest priority if necessary */
/* BLASTK_priomask should be non-zero */
static inline void BLASTK_lowprio_notify()
{
	u32_t prio;
	u32_t hthread;
	BLASTK_thread_context *tmp;
	prio = BLASTK_runlist_worst_prio();
	tmp = BLASTK_runlist[prio];
	hthread = BLASTK_runlist[prio]->hthread;
	BLASTK_priomask |= 1<<hthread;
	change_imask(hthread,0);
}

/* Notify the current lowprio thread that he is no longer the worst */
/* Note that if threads are waiting, this should not be called, you are
 * going to get into trouble if you raise the priority of a waiting thread */
static inline void BLASTK_raise_lowprio()
{
	u32_t mask = BLASTK_priomask;
	if (BLASTK_wait_mask) return; // just a sanity check... should be an error
	BLASTK_priomask = 0;
	change_imask(Q6_R_ct0_R(mask),-1);
}

#endif


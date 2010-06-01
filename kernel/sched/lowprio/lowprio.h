/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef LOWPRIO_H
#define LOWPRIO_H 1

#include <runlist.h>
#include <q6protos.h>
#include <hw.h>
#include <globals.h>

/* Notify a low priority thread that it is the new lowest priority if necessary */
/* H2K_gp->priomask should be non-zero */
static inline void H2K_lowprio_notify()
{
	u32_t prio;
	u32_t hthread;
	prio = H2K_runlist_worst_prio();
	hthread = H2K_gp->runlist[prio]->hthread;
	H2K_gp->priomask |= 1<<hthread;
	change_imask(hthread,0);
}

/* Notify the current lowprio thread that he is no longer the worst */
/* Note that if threads are waiting, this should not be called, you are
 * going to get into trouble if you raise the priority of a waiting thread */
static inline void H2K_raise_lowprio()
{
	u32_t mask = H2K_gp->priomask;
	if (H2K_gp->wait_mask) return; // just a sanity check... should be an error
	H2K_gp->priomask = 0;
	change_imask(Q6_R_ct0_R(mask),-1);
}

void H2K_lowprio_init() IN_SECTION(".text.init.lowprio");

#endif


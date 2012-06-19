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
#include <resched.h>
#include <popup.h>
#include <globals.h>
#include <switch.h>
#include <max.h>
#include <intcontrol.h>

void H2K_popup_int(u32_t intnum, H2K_thread_context *me, u32_t hwtnum, H2K_thread_context *woken)
{
	BKL_LOCK(&H2K_bkl);
	H2K_gp->inthandlers[intnum].raw = 0;
	if (unlikely(woken == NULL)) {
		/* Auto-disabled, now we should re-post interrupt, I think */
		BKL_UNLOCK(&H2K_bkl);
		return;
	}
	if (me != NULL) {
		H2K_runlist_remove(me);
		H2K_ready_append(me);
	} else {
		H2K_gp->wait_mask = Q6_R_clrbit_RR(H2K_gp->wait_mask,hwtnum);
	}
	/* Assume woken is better than interrupted thread.  
	 * If not, check_sanity will detect. 
	 * Let check_sanity find the new lowprio thread also...
	 */
	H2K_gp->priomask = Q6_R_clrbit_RR(H2K_gp->priomask,hwtnum);
	highprio_imask(hwtnum);
	H2K_runlist_push(woken);
	H2K_switch(me,woken);
}

int H2K_popup_wait(u32_t intnum, H2K_thread_context *me)
{
	int hthread = me->hthread;
	if (intnum >= MAX_INTERRUPTS) return -1;
#if ARCHV >= 4
	/* Can't change L2 interrupt vector */
	if (intnum == 31) return -1;
#endif
	BKL_LOCK(&H2K_bkl);
	if (H2K_gp->inthandlers[intnum].param != NULL) {
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}
	H2K_gp->inthandlers[intnum].param = me;
	H2K_gp->inthandlers[intnum].handler = H2K_popup_int;
	H2K_runlist_remove(me);
	me->status = H2K_STATUS_INTBLOCKED;
	me->r0100 = intnum;
	H2K_intcontrol_enable(intnum);
	H2K_dosched(me,hthread);
}

/* NOTE: must be called with bkl held */
void H2K_popup_cancel(H2K_thread_context *dest)
{
	u32_t intnum = dest->r00;
	H2K_gp->inthandlers[intnum].raw = 0;
	dest->r00 = -1;
	/* intcontrol_disable intnum? */
}


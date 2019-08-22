/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <intpool.h>
#include <vmblock.h>
#include <max.h>
#include <hw.h>
#include <intcontrol.h>
#include <ring.h>
#include <runlist.h>
#include <readylist.h>
#include <switch.h>
#include <dosched.h>

/*
 * EJP: FIXME: Should there be a single intpool for a vmblock? 
 * Maybe we should allow multiple intpools?
 * The second argument could point to an intpool struct instead of a vmblock.
 * This might make it more efficient to support both qurt interrupts and fastint replacements with intpool
 */
void H2K_intpool_int(u32_t intnum, H2K_thread_context *me, u32_t hwtnum, H2K_vmblock_t *vmblock)
{
	H2K_thread_context *woken;
	BKL_LOCK(&H2K_bkl);
	if ((woken = vmblock->intpool) == NULL) {
		/* FIXME: no threads ready, soft-pend interrupt for now */
		vmblock->intpool_pending[intnum>>5] |= (1<<(intnum & 0x1f));
		vmblock->intpool_anypending = 1;
		BKL_UNLOCK(&H2K_bkl);
		return;
	}
	H2K_ring_remove(&vmblock->intpool,woken);
	woken->r00 = intnum;
	if (me != NULL) {
		H2K_runlist_remove(me);
		H2K_ready_append(me);
	} else {
		H2K_gp->wait_mask = Q6_R_clrbit_RR(H2K_gp->wait_mask,hwtnum);
	}
	H2K_gp->priomask = Q6_R_clrbit_RR(H2K_gp->priomask,hwtnum);
	highprio_imask(hwtnum);
	H2K_runlist_push(woken);
	H2K_switch(me,woken);
}

static inline int get_pending_interrupt(H2K_vmblock_t *vmblock)
{
	int i;
	int intno = -1;
	u32_t word;
	int bitidx = 0;
	for (i = 0; i < MAX_INTERRUPTS/32; i++) {
		if ((word = vmblock->intpool_pending[i]) != 0) {
			bitidx = Q6_R_ct0_R(word);
			word &= ~(1<<bitidx); // clear the bit
			vmblock->intpool_pending[i] = word;
			intno = (i<<5) | bitidx;
			break;
		}
	}
	return intno;
}

int H2K_intpool_wait(u32_t int_ack_num, H2K_thread_context *me)
{
	int hthread = me->hthread;
	int intno;
	H2K_vmblock_t *vmblock = me->vmblock;
	if (int_ack_num < MAX_INTERRUPTS) {
		H2K_intcontrol_enable(int_ack_num);
	}
	BKL_LOCK(&H2K_bkl);
	if (unlikely(vmblock->intpool_anypending)) {
		if ((intno = get_pending_interrupt(vmblock)) >= 0) {
			BKL_UNLOCK(&H2K_bkl);
			return intno;
		} else {
			/* Not really pending any more */
			vmblock->intpool_anypending = 0;
		}
	}
	/* FIXME: check pending intpool interrupts */
	H2K_runlist_remove(me);
	H2K_ring_append(&vmblock->intpool,me);
	me->status = H2K_STATUS_INTBLOCKED;	/* OR INTPOOL_BLOCKED? */
	me->r00 = -1;
	H2K_dosched(me,hthread);
}

int H2K_intpool_configure(u32_t intno, u32_t enable, H2K_thread_context *me)
{
	BKL_LOCK(&H2K_bkl);
	if (enable) {
		H2K_gp->inthandlers[intno].param = me->vmblock;
		H2K_gp->inthandlers[intno].handler = H2K_intpool_int;
		H2K_intcontrol_enable(intno);
	} else {
		H2K_intcontrol_disable(intno);
		H2K_gp->inthandlers[intno].raw = 0;
	}
	BKL_UNLOCK(&H2K_bkl);
	return 0;
}

/* NOTE: must be called with bkl held */
/* Can this be joined with H2K_popup_cancel? */
void H2K_intpool_cancel(H2K_thread_context *dest)
{
	H2K_vmblock_t *vmblock = dest->vmblock;
	H2K_ring_remove(&vmblock->intpool,dest);
	dest->r00 = -1;
}


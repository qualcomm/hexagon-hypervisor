/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vm.h>
#include <vmdefs.h>
#include <max.h>
#include <vmevent.h>
#include <vmint.h>
#include <fatal.h>

void H2K_vm_event(u32_t gbadva, u32_t cause, u32_t vec_offset, H2K_thread_context *me)
{
	u32_t tmp;
	if (me->gevb == 0) return H2K_fatal_thread(-3,me,0,0,me->hthread);
	me->gelr = me->elr;
	me->elr = ((u32_t)me->gevb) + vec_offset;
	me->gbadva = gbadva;
	if (me->ssr_guest == 0) {
		tmp = me->r29;
		me->ssr_guest = 1;
		me->r29 = me->gosp;
		me->gosp = tmp;
		me->gssr = H2K_GSSR_UM | cause;
	} else {
		me->gssr = cause;
	}

	/* save IE status and disable */
	if (me->vmstatus & H2K_VMSTATUS_IE) {
		me->gssr |= H2K_GSSR_IE;
	} else {
		me->gssr &= ~H2K_GSSR_IE;
	}

#if __QDSP6_ARCH__ >= 4
	/* Save Single Step status and disable */
	if (me->ssr_ss) {
		me->gssr |= H2K_GSSR_SS;
		me->ssr_ss = 0;
	} else {
		me->gssr &= ~H2K_GSSR_SS;
	}
#endif
	H2K_disable_guest_interrupts(me);
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vmint.h>
#include <vmwork.h>
#include <vmevent.h>
#include <stop.h>
#include <context.h>
#include <atomic.h>

s32_t H2K_vm_do_work(H2K_thread_context *me)
{
	s32_t intno;
	if (me->vmstatus & H2K_VMSTATUS_KILL) {
		/* This thread must die */
		H2K_thread_stop(me);
	}
	if (me->vmstatus & H2K_VMSTATUS_IE) {
		/* Try to get interrupt */
		intno = H2K_vm_interrupt_get(me->vmblock, me->vmcpu);
		if (intno < 0) {
			/* No interrupt, nothing to do */
			H2K_atomic_clrbit(&me->atomic_status_word,H2K_VMSTATUS_VMWORK_BIT);
		}
		else {
			/* Interrupts enabled, interrupt pulled from controller.  Do interrupt! */
			H2K_vm_event(0,intno,INTERRUPT_GEVB_OFFSET,me);
		}
		return intno;
	} else {
		/* Someone asked me to do work, but interrupts are disabled
		 * now, so I can't. */
		/* Keep VMWORK bit set so I can do the work later */
	}
}


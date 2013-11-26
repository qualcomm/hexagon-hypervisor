/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vmfuncs.h>
#include <vmdefs.h>
#include <max.h>
#include <vmint.h>
#include <atomic.h>
#include <stop.h>
#include <yield.h>
#include <asid.h>
#include <time.h>
#include <stlb.h>
#include <tlbmisc.h>
#include <globals.h>
#include <hw.h>
#include <id.h>
#include <thread.h>
#include <create.h>
#include <runlist.h>
#include <fatal.h>
#include <vmint.h>
#include <dosched.h>
#include <vmwork.h>

void H2K_vmtrap_version(H2K_thread_context *me)
{
	me->r00 = H2K_VM_VERSION;
}

/* 1 */
void H2K_vmtrap_return(H2K_thread_context *me)
{
	u32_t tmp;
	u32_t ints_enabled;
	H2K_gregs_save(me);
	if (me->gssr & H2K_GSSR_UM) {
		/* Swap stack pointers */
		tmp = me->r29;
		me->r29 = me->gosp;
		me->gosp = tmp;
		me->ssr &= (~(1<<SSR_GUEST_BIT));
	}
	me->elr = me->gelr;
	H2K_set_elr(me->elr);

#if ARCHV >= 4
	/* If guest wants to single step, set in SSR */
	if (me->gssr & H2K_GSSR_SS) {
		me->ssr_ss = 1;
	} else {
		me->ssr_ss = 0;
	}
#endif
	ints_enabled = (me->gssr & H2K_GSSR_IE);
	H2K_gregs_restore(me);

	/* if guest had interrupts enabled, enable in guest status */
	if (ints_enabled) {
		/* NOTE: this might cause a vmevent if interrupts are pending */
		H2K_enable_guest_interrupts(me);
	}

}

/* 2 */
void H2K_vmtrap_setvec(H2K_thread_context *me)
{
	me->gevb = (void *)((u32_t)(me->r00));
	me->r00 = 0;
}

/* 3 */
void H2K_vmtrap_setie(H2K_thread_context *me)
{
	u32_t prev;
	if (me->r00 & 0x1) {
		prev = H2K_enable_guest_interrupts(me);
	} else {
		prev = H2K_disable_guest_interrupts(me);
	}
	me->r00 = prev;
}

/* 4 */
void H2K_vmtrap_getie(H2K_thread_context *me)
{
	me->r00 = ((me->atomic_status_word & H2K_VMSTATUS_IE) != 0);
}

/* 14 */
void H2K_vmtrap_get_pcycles(H2K_thread_context *me)
{
	/* Return current cpu time */
	me->r0100 = H2K_cputime_get(me);
	me->r0302 = H2K_pcycles_get(me);
}

/* 15 -- REMOVE */
void H2K_vmtrap_set_pcycles(H2K_thread_context *me)
{
	/* Set accumulated pcycles to specified amount, and then set
	 * oncpu_start to current pcycles */
	me->totalcycles = me->r0100;
	H2K_gp->oncpu_start[get_hwtnum()] = H2K_get_pcycle_reg();
}

/* 16 */
void H2K_vmtrap_wait(H2K_thread_context *me)
{
/* Wait for interrupt... or fall through if interrupts disabled and
 * something pending */

	s32_t intno;

	BKL_LOCK();
	intno = H2K_vm_do_work(me);

	if (intno == -1) {
		/* nothing pending; wait  */
		me->status = H2K_STATUS_VMWAIT;

		/* for first 64 cpus we track waiting: deliver shared ints to them first */
		if (me->id.cpuidx < sizeof(long_bitmask_t) * 8) {
			me->vmblock->waiting_cpus |= (0x1 << me->id.cpuidx);
		}
		H2K_runlist_remove(me);
		H2K_dosched(me,me->hthread);
	} else {
		/* Interrupt pending; either it was taken or interrupts are disabled.  In
			 either case vmwait returns the interrupt number. */

		me->r00 = intno;
		BKL_UNLOCK();
	}
}

/* 17 */
void H2K_vmtrap_yield(H2K_thread_context *me)
{
	H2K_sched_yield(me);
	me->r00 = 0;
}

/* 18 */
void H2K_vmtrap_start(H2K_thread_context *me)
{
	/* FIXME: need to pass arg1?  use vmblock bestprio instead of base_prio? */
	                               /*      pc       sp  arg1 */
	me->r00 = H2K_thread_create_no_squash(me->r00, me->r01, 0, me->base_prio, me->vmblock, me);
}

/* 19 */
void H2K_vmtrap_stop(H2K_thread_context *me)
{
	/* Destroy, or just make blocked? */
	H2K_thread_stop(me->r00, me);
}

/* 20 */
void H2K_vmtrap_vmpid(H2K_thread_context *me)
{
	me->r00 = H2K_id_from_context(me).raw;
}

/* 21 */
void H2K_vmtrap_setregs(H2K_thread_context *me)
{
	me->gssr_gelr = me->r0100;
	me->gbadva_gosp = me->r0302;
	H2K_gregs_restore(me);
}

/* 22 */
void H2K_vmtrap_getregs(H2K_thread_context *me)
{
	H2K_gregs_save(me);
	me->r0100 = me->gssr_gelr;
	me->r0302 = me->gbadva_gosp;
}


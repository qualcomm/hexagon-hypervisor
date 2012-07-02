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
	if (me->gssr & H2K_GSSR_UM) {
		/* Swap stack pointers */
		tmp = me->r29;
		me->r29 = me->gosp;
		me->gosp = tmp;
		me->ssr &= (~(1<<SSR_GUEST_BIT));
	}
	me->elr = me->gelr;

	/* if guest had interrupts enabled, enable in guest status */
	if (me->gssr & H2K_GSSR_IE) {
		H2K_enable_guest_interrupts(me);
	}

#if ARCHV >= 4
	/* If guest wants to single step, set in SSR */
	if (me->gssr & H2K_GSSR_SS) {
		me->ssr_ss = 1;
	}
#endif
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

/* 10 */
void H2K_vmtrap_clrmap(H2K_thread_context *me)
{
	/* Invalidate HW/STLB entry */
	u32_t va, count;
	va = me->r00;
	count = me->r01;

	me->r00 = 0;

	if (count == 0) { // FIXME: error?
		// me->r00 = -1;
		return;
	}
	/* FIXME: temporary hack */
	/* H2K_mem_stlb_invalidate_va(va, count, me->ssr_asid, me); */
	/* H2K_mem_tlb_invalidate_va(va, count, me->ssr_asid, me); */
	H2K_mem_stlb_invalidate_asid(me->ssr_asid);
	H2K_mem_tlb_invalidate_asid(me->ssr_asid);
}

/* 11 */
void H2K_vmtrap_newmap(H2K_thread_context *me)
{
	s32_t newasid;
	u32_t newptb = me->r00;
	translation_type type;
	tlb_invalidate_flag flag = me->r02;

	
	/* Don't allow guest to newmap offset translations for now.  FIXME?  Can
		 maybe store offset descriptor in asid table */
	if (me->r01 >= H2K_ASID_TRANS_TYPE_XXX_LAST) { // bad type
		me->r00 = -1;
		return;
	}

	type = me->r01;
	if ((newasid = H2K_asid_table_inc(newptb, type, flag, me->vmblock)) == -1) {
		me->r00 = -1;
	} else {
		H2K_asid_table_dec(me->ssr_asid);
		me->ssr_asid = newasid;
		me->r00 = 0;
	}
}

/* 13 */
void H2K_vmtrap_cachectl(H2K_thread_context *me)
{
	/* FIXME: doing this like minivm for now */

	cacheop_type op = (cacheop_type)me->r00;
	// u32_t va = me->r01;
	// u32_t count = me->r02;
	u32_t i, j;
	u32_t idx = 0;

	me->r00 = 0;

	/* only ickill on simulator, for speed */
	if (H2K_gp->on_simulator) {
		if (op == H2K_CACHECTL_ICKILL || op == H2K_CACHECTL_ICINVA) {
			asm volatile
				(
				 " ickill \n"
				 );
		}
		return;
	}

	if (op >= H2K_CACHECTL_BADOP) {
		me->r00 = -1;
		return;
	}

	switch(op) {
	case H2K_CACHECTL_ICKILL:
	case H2K_CACHECTL_ICINVA:
		asm volatile
			(
			 " ickill \n"
			 );
		return;

	case H2K_CACHECTL_DCKILL:
	case H2K_CACHECTL_DCCLEANINVA:
		for (i = 0; i < CACHEIDX_MAX; i++) {
			asm volatile
				(
				 "dccleaninvidx(%0) \n"
				 :
				 : "r"(i)
				 );
		};
		return;

	case H2K_CACHECTL_IDSYNC:
		for (i = 0; i < WAYS_MAX; i++) {
			for (j = 0; j < (SETS_MAX * 32); j += 32) {
				idx += i + j;
				asm volatile
					(
					 "icinvidx(%0) \n"
					 "dccleanidx(%0) \n"
					 :
					 : "r"(idx)
					 );
			}
		}
		return;

	case H2K_CACHECTL_L2KILL:
	case H2K_CACHECTL_BADOP:  // shut up warning
		return;
	}
}

/* 14 */
void H2K_vmtrap_get_pcycles(H2K_thread_context *me)
{
	/* Return current cpu time */
	me->r0100 = H2K_cputime_get(me);
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
	H2K_thread_stop(me);
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
}

/* 22 */
void H2K_vmtrap_getregs(H2K_thread_context *me)
{
	me->r0100 = me->gssr_gelr;
	me->r0302 = me->gbadva_gosp;
}


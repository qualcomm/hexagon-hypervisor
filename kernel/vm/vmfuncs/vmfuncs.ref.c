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
#include <thread.h>
#include <create.h>
#include <vmwork.h>

u32_t H2K_enable_guest_interrupts(H2K_thread_context *me) {
	u32_t prev = !(H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT));
	
	if (!prev  // interrupts were disabled, try to take now
			&& (me->vmstatus & H2K_VMSTATUS_VMWORK)) H2K_vm_do_work(me);
	return prev;
}

u32_t H2K_disable_guest_interrupts(H2K_thread_context *me) {
	return H2K_atomic_clrbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT);
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
	me->r00 = ((me->atomic_status_word & H2K_VMSTATUS_IE_BIT) != 0);
}

#if 0
void H2K_vmtrap_swi(H2K_thread_context *me)
{
	H2K_vm_interrupt_post(me->vmblock,me->vmcpu,(u32_t)(me->r00));
}

void H2K_vmtrap_iack(H2K_thread_context *me)
{
	H2K_vm_interrupt_enable(me->vmblock,(u32_t)(me->r00));
}

void H2K_vmtrap_setimask(H2K_thread_context *me)
{
	/* CHange interface */
}

void H2K_vmtrap_getimask(H2K_thread_context *me)
{
	/* Change interface */
}

void H2K_vmtrap_iconf(H2K_thread_context *me)
{
	/* Change interface? */
}
#endif

/* 10 */
void H2K_vmtrap_clrmap(H2K_thread_context *me)
{
	/* Invalidate HW/STLB entry */
	u32_t va;
	va = me->r00;
	H2K_mem_stlb_invalidate_va(va,me->ssr_asid,me);
	H2K_mem_tlb_invalidate_va(va,me->ssr_asid,me);
	me->r00 = 0;
}

/* 11 */
void H2K_vmtrap_newmap(H2K_thread_context *me)
{
	s32_t newasid;
	u32_t newptb = me->r00;
	translation_type type = me->r01;

	/* type is checked in H2K_asid_table_inc */
	if ((newasid = H2K_asid_table_inc(newptb, type)) < 0) {
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
	/* Do various cache control things */
}

/* 14 */
void H2K_vmtrap_get_pcycles(H2K_thread_context *me)
{
	/* Return current cpu time */
	me->r0100 = H2K_cputime_get(me);
}

/* 15 */
void H2K_vmtrap_set_pcycles(H2K_thread_context *me)
{
	/* Set accumulated pcycles to specified amount, and then set
	 * oncpu_start to current pcycles */
	me->totalcycles = me->r0100;
	H2K_gp->oncpu_start[get_hwtnum()] = H2K_pcycles_get(me);
}

/* 16 */
void H2K_vmtrap_wait(H2K_thread_context *me)
{
	/* Wait for interrupt... or fall through if interrupts disabled and
	 * something pending */
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
	/* Create, or just unblock? */

	s32_t ret;
	/* FIXME: need to pass arg1?  use vmblock bestprio instead of base_prio? */
	                               /*      pc       sp  arg1 */
	ret = H2K_thread_create_no_squash(me->r00, me->r01, 0, me->base_prio, me->vmblock, me);
	if (ret < 0) { //error
		me->r00 = ret;
	}
	else {
		me->r00 = ((H2K_thread_context *)ret)->vmcpu;
	}
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
	me->r00 = me->vmcpu;
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


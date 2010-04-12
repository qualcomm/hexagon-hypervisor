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
#include <stlb.h>
#include <tlbmisc.h>

void H2K_vmtrap_return(H2K_thread_context *me)
{
	u32_t tmp;
	if (((me->gssr >> 31) & 1) != 0) {
		/* Swap stack pointers */
		tmp = me->r29;
		me->r29 = me->gosp;
		me->gosp = tmp;
		me->ssr &= (~(1<<SSR_GUEST_BIT));
	}
	me->elr = me->gelr;
}

void H2K_vmtrap_setvec(H2K_thread_context *me)
{
	me->gevb = (void *)((u32_t)(me->r0100));
	me->r0100 = 0;
}
void H2K_vmtrap_setie(H2K_thread_context *me)
{
	if (me->r0100 & 0x1) {
		H2K_atomic_setbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT);
	} else {
		H2K_atomic_clrbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT);
	}
	me->r0100 = 0;
}

void H2K_vmtrap_getie(H2K_thread_context *me)
{
	me->r0100 = ((me->atomic_status_word & H2K_VMSTATUS_IE_BIT) != 0);
}

void H2K_vmtrap_swi(H2K_thread_context *me)
{
	H2K_vm_interrupt_post(me->vmblock,me->vmcpu,(u32_t)(me->r0100));
}

void H2K_vmtrap_iack(H2K_thread_context *me)
{
	H2K_vm_interrupt_enable(me->vmblock,(u32_t)(me->r0100));
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

void H2K_vmtrap_clrmap(H2K_thread_context *me)
{
	/* Invalidate HW/STLB entry */
	u32_t va;
	va = me->r0100;
	H2K_mem_stlb_invalidate_va(va,me->ssr_asid,me);
	H2K_mem_tlb_invalidate_va(va,me->ssr_asid,me);
	me->r0100 = 0;
}

void H2K_vmtrap_register_ptb(H2K_thread_context *me)
{
	s32_t newasid;
	u32_t newptb = me->r0100;
	if ((newasid = H2K_asid_table_inc(newptb)) < 0) {
		me->r0100 = -1;
	} else {
		H2K_asid_table_dec(me->ssr_asid);
		me->ssr_asid = newasid;
	}
}

void H2K_vmtrap_cachectl(H2K_thread_context *me)
{
}

void H2K_vmtrap_get_pcycles(H2K_thread_context *me)
{
}

void H2K_vmtrap_set_pcycles(H2K_thread_context *me)
{
}

void H2K_vmtrap_wait(H2K_thread_context *me)
{
}

void H2K_vmtrap_yield(H2K_thread_context *me)
{
	H2K_sched_yield(me);
	me->r0100 = 0;
}

void H2K_vmtrap_start(H2K_thread_context *me)
{
	/* Create, or just unblock? */
	me->r0100 = 0;
}

void H2K_vmtrap_stop(H2K_thread_context *me)
{
	/* Destroy, or just make blocked? */
	H2K_thread_stop(me);
	me->r0100 = 0;
}

void H2K_vmtrap_vmpid(H2K_thread_context *me)
{
	me->r0100 = me->vmcpu;
}


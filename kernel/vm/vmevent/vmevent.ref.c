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

static inline void H2K_mem_pagefault_save_gregs(H2K_thread_context *me)
{
#if __QDSP6_ARCH__ >= 4
	u64_t g10;
	u64_t g32;
	__asm__ ( " %0 = g1:0\n %1 = g3:2\n" : "=r"(g10),"=r"(g32));
	me->gssr_gelr = g10;
	me->gbadva_gosp = g32;
#endif
}

static inline void H2K_mem_event_restore_gregs(H2K_thread_context *me)
{
#if __QDSP6_ARCH__ >= 4
	u64_t g10;
	u64_t g32;
	g10 = me->gssr_gelr;
	g32 = me->gbadva_gosp;
	asm volatile ( " g1:0 = %0\n g3:2 = %1\n" : : "r"(g10),"r"(g32));
#endif
}

void H2K_vm_event(u32_t gbadva, u32_t cause, u32_t vec_offset, H2K_thread_context *me)
{
	u32_t tmp;
	me->gelr = me->ssrelr;
	me->elr = ((u32_t)me->gevb) + vec_offset;
	me->gbadva = gbadva;
	if ((me->ssr & (1<<SSR_GUEST_BIT)) == 0) {
		tmp = me->r29;
		me->ssr |= (1<<SSR_GUEST_BIT);
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

	H2K_disable_guest_interrupts(me);
	H2K_mem_event_restore_gregs(me);
}


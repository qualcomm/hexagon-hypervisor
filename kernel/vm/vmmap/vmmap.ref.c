/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vmmap.h>
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
	translation_type type = me->r01;
	tlb_invalidate_flag flag = me->r02;

	/* type is checked in H2K_asid_table_inc */
	if ((newasid = H2K_asid_table_inc(newptb, type, flag, me->vmblock)) < 0) {
		me->r00 = -1;
	} else {
		H2K_asid_table_dec(me->ssr_asid);
		me->ssr_asid = newasid;
		me->r00 = 0;
	}
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <context.h>
#include <pagefault.h>
#include <globals.h>
#include <tlbfmt.h>
#include <stlb.h>
#include <linear.h>
#include <pagewalk.h>
#include <asid.h>
#include <tlbfill.h>

#if __QDSP6_ARCH__ >= 4
static inline u32_t H2K_mem_tlb_v3_user_check(H2K_thread_context *me) { return 0; }
#else
static inline u32_t H2K_mem_tlb_v3_user_check(H2K_thread_context *me)
{
	return 0;
}
#endif

static inline void H2K_mem_tlb_insert(H2K_mem_tlbfmt_t entry, H2K_thread_context *me)
{
	u64_t rawentry;
	u32_t index = H2K_gp->tlb_index;
	if ((index+1) < MAX_TLB_ENTRIES) {
		H2K_gp->tlb_index = index+1;
	} else {
		H2K_gp->tlb_index = TLB_FIRST_REPLACEABLE_ENTRY;
	}
#if __QDSP6_ARCH__ <= 3
	/* set guest bit in the ASID if this was a guest miss */
	if (me->ssr_guest) entry.guestonly = 1;
	rawentry = entry.raw;
	asm volatile (
	" tlbhi = %H0\n"
	" tlblo = %L0\n"
	" tlbidx = %1\n" 
	" tlbw\n" : :"r"(rawentry),"r"(index));
#else
	rawentry = entry.raw;
	asm volatile (" tlbw(%0,%1)\n" : : "r"(rawentry),"r"(index));
#endif
}

void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me)
{
	H2K_mem_tlbfmt_t entry;
	u32_t asid = me->ssr_asid;
	H2K_mem_tlbfmt_t (*get_fn)(u32_t badva, H2K_thread_context *me);
	if ((entry = H2K_mem_stlb_lookup(va,asid,me)).raw != 0) {
		if (H2K_mem_tlb_v3_user_check(me)) return;
		H2K_mem_tlb_insert(entry,me);
		return;
	}
	switch (H2K_mem_asid_table[asid].fields.transtype) {
	case H2K_ASID_TRANS_TYPE_LINEAR:
		get_fn = H2K_mem_get_linear;
		break;

	case H2K_ASID_TRANS_TYPE_TABLE:
		get_fn = H2K_mem_get_pagetable;
		break;

	case H2K_ASID_TRANS_TYPE_OFFSET:
		get_fn = H2K_vm_get_offset;
		break;

	default:
		return;
	}

	if ((entry = get_fn(va,me)).raw != 0) {
		if (H2K_mem_tlb_v3_user_check(me)) return;
		H2K_mem_stlb_add(va,asid,entry,me);
		H2K_mem_tlb_insert(entry,me);
		return;
	}
	return H2K_mem_pagefault(va,me);
}


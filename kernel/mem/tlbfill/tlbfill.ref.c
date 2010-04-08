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

#if __QDSP6_ARCH__ >= 4
static inline u32_t H2K_mem_tlb_v3_user_check(H2K_thread_context *me) { return 0; }
#else
static inline u32_t H2K_mem_tlb_v3_user_check(H2K_thread_context *me)
{
	return 0;
}
#endif

static inline void H2K_mem_tlb_insert(u64_t entry, H2K_thread_context *me)
{
	u32_t index = H2K_gp->tlb_index;
	if ((index+1) > MAX_TLB_ENTRIES) {
		H2K_gp->tlb_index = TLB_FIRST_REPLACEABLE_ENTRY;
	} else {
		H2K_gp->tlb_index = index+1;
	}
#if __QDSP6_ARCH__ <= 3
	if (me->ssr_fake_guest) entry |= 0x0200000000000000ULL;
	asm volatile (
	" tlbhi = %H0\n"
	" tlblo = %L0\n"
	" tlbidx = %1\n" 
	" tlbw\n" : :"r"(entry),"r"(index));
#else
	asm volatile (" tlbw(%0,%1)\n" : : "r"(entry),"r"(index));
#endif
}

void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me)
{
	H2K_mem_tlbfmt_t entry;
	u32_t asid = me->ssr_asid;
	if ((entry = H2K_mem_stlb_lookup(va,asid,me)).raw != 0) {
		if (H2K_mem_tlb_v3_user_check(me)) return;
		H2K_mem_tlb_insert(entry.raw,me);
		return;
	}
	if ((entry = H2K_mem_translate_linear(va,me)).raw != 0) {
		if (H2K_mem_tlb_v3_user_check(me)) return;
		H2K_mem_stlb_add(va,asid,entry,me);
		H2K_mem_tlb_insert(entry.raw,me);
		return;
	}
	return H2K_mem_pagefault(va,me);
}


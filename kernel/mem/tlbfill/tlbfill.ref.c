/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <context.h>
#include <pagefault.h>

static u32_t H2K_tlb_index = FIRST_REPLACEABLE_ENTRY;

static inline void H2K_mem_tlb_insert(u64_t entry)
{
	u32_t index = H2K_tlb_index;
	if ((index+1) > MAX_TLB_ENTRIES) {
		H2K_tlb_index = FIRST_REPLACEABLE_ENTRY;
	} else {
		H2K_tlb_index = index+1;
	}
#if __QDSP6_ARCH__ <= 3
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
	u64_t entry;
	u32_t asid = me->ssr_asid;
	if ((entry = H2K_mem_stlb_lookup(va,asid,me)) != 0) {
		H2K_mem_tlb_insert(entry);
		return;
	}
	if ((entry = H2K_mem_translate_linear(va,me)) != 0) {
		H2K_mem_tlb_insert(entry);
		H2K_mem_stlb_add(va,asid,entry,me);
		return;
	}
	return H2K_mem_pagefault(me);
}


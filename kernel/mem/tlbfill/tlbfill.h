/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBFILL_H
#define H2K_TLBFILL_H 1

#include <symbols.h>

void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me) IN_SECTION(".text.mem.tlb");

static inline void H2K_mem_tlb_insert(H2K_mem_tlbfmt_t entry, H2K_thread_context *me)
{
	u64_t rawentry;
	u32_t index = H2K_gp->tlb_index;
	if ((index+1) < MAX_TLB_ENTRIES) {
		H2K_gp->tlb_index = index+1;
	} else {
		H2K_gp->tlb_index = ((u32_t)&TLB_LAST_KERNEL_ENTRY) + 1;
	}
#if ARCHV <= 3
	/* set guest bit in the ASID if this was a guest miss */
	if (me->ssr_guest) entry.guestonly = 1;
	rawentry = entry.raw;
	asm volatile
		(
		 " tlbhi = %H0\n"
		 " tlblo = %L0\n"
		 " tlbidx = %1\n" 
		 " tlbw\n"
		 " isync\n"
		 : :"r"(rawentry),"r"(index));
#else
	rawentry = entry.raw;
	asm volatile
		(
		 " tlbw(%0,%1)\n"
		 " isync\n"
		 : : "r"(rawentry),"r"(index));
#endif
}

#endif


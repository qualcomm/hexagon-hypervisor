/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <max.h>
#include <hw.h>
#include <symbols.h>
#include <asid.h>
#include <linear.h>
#include <pagewalk.h>
#include <globals.h>

void H2K_mem_tlb_invalidate_asid(u32_t asid)
{
#if ARCHV >= 5
	asid <<=  (32 - PAGE_BITS);
	asm volatile
		(
		 " tlbinvasid(%0)\n"
		 :
		 : "r"(asid)
		 );

#else
	u32_t i;
	u64_t tmp,mask,check;
	mask = ((u64_t)(MAX_ASIDS - 1)) << (32+20);
	check = (((u64_t)(asid)) << (32+20)) & mask;
	for (i = 0; i <= H2K_gp->last_tlb_index; i++) {

		H2K_TLB_ATOMIC_START;
		tmp = H2K_mem_tlb_read(i);
		H2K_TLB_ATOMIC_END;

		if ((tmp & mask) == (check)) {
			H2K_TLB_ATOMIC_START;
			H2K_mem_tlb_write(i,0);
			H2K_TLB_ATOMIC_END;
		}
	}
#endif
	H2K_isync();
}

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me)
{
	int tmp;
	if (count > 1) return H2K_mem_tlb_invalidate_asid(asid);
	H2K_TLB_ATOMIC_START;
	tmp = H2K_mem_tlb_probe(va,asid);
	if ((tmp >= 0) && (tmp <= H2K_gp->last_tlb_index)) {
		H2K_mem_tlb_write(tmp,0);
		H2K_isync();
	}
	H2K_TLB_ATOMIC_END;
}

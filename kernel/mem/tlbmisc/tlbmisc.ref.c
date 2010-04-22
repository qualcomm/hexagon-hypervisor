/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <tlbmisc.h>
#include <max.h>

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me)
{
	u32_t tmp;
	tmp = H2K_mem_tlb_probe(va,asid);
	if (((tmp >> 31) & 1) == 0) {
		H2K_mem_tlb_write(tmp,0);
	}
#if __QDSP6_ARCH__ <= 3
	/* For V3 and earlier, also need to probe fake guest bit */
	tmp = H2K_mem_tlb_probe(va,asid|0x20);
	if (((tmp >> 31) & 1) == 0) {
		H2K_mem_tlb_write(tmp,0);
	}
#endif
}

void H2K_mem_tlb_invalidate_asid(u32_t asid)
{
	u32_t i;
	u64_t tmp,mask,check;
	mask = ((u64_t)(MAX_TLB_ENTRIES-1)) << (32+20);
	check = (((u64_t)(asid)) << (32+20)) & mask;
	for (i = TLB_FIRST_REPLACEABLE_ENTRY; i < MAX_TLB_ENTRIES; i++) {
		tmp = H2K_mem_tlb_read(i);
		if ((tmp & mask) == (check)) {
			H2K_mem_tlb_write(i,0);
		}
	}
}


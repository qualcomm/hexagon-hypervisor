/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <tlbmisc.h>
#include <max.h>
#include <hw.h>
#include <symbols.h>

void H2K_mem_tlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me)
{
	u32_t tmp;
	u32_t start = va >> PAGE_BITS;
	u32_t end = (va + count - 1) >> PAGE_BITS;
	u32_t page;

	/* FIXME: this needs to be smarter: Use relevant page size */
	/* If the count is large, better to read the whole TLB and
	 check each entry instead of probing all the pages */

	for (page = start; page <= end; page++) {
		H2K_TLB_ATOMIC_START;
		tmp = H2K_mem_tlb_probe(page << PAGE_BITS, asid);
		H2K_TLB_ATOMIC_END;
		if (((tmp >> 31) & 1) == 0) {
			H2K_TLB_ATOMIC_START;
			H2K_mem_tlb_write(tmp,0);
			H2K_TLB_ATOMIC_END;
		}
#if ARCHV <= 3
		/* For V3 and earlier, also need to probe fake guest bit */
		tmp = H2K_mem_tlb_probe(page << PAGE_BITS, asid|0x20);
		if (((tmp >> 31) & 1) == 0) {
			H2K_TLB_ATOMIC_START;
			H2K_mem_tlb_write(tmp,0);
			H2K_TLB_ATOMIC_END;
		}
#endif
		H2K_isync();
	}
}

void H2K_mem_tlb_invalidate_asid(u32_t asid)
{
	u32_t i;
	u64_t tmp,mask,check;
	mask = ((u64_t)(MAX_ASIDS - 1)) << (32+20);
	check = (((u64_t)(asid)) << (32+20)) & mask;
	for (i = ((u32_t)&TLB_LAST_KERNEL_ENTRY) + 1; i < MAX_TLB_ENTRIES; i++) {

		H2K_TLB_ATOMIC_START;
		tmp = H2K_mem_tlb_read(i);
		H2K_TLB_ATOMIC_END;

		if ((tmp & mask) == (check)) {
			H2K_TLB_ATOMIC_START;
			H2K_mem_tlb_write(i,0);
			H2K_TLB_ATOMIC_END;
		}
	}
	H2K_isync();
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <tlbfmt.h>
#include <hw.h>
#include <symbols.h>
#include <physread.h>
#include <tmpmap.h>

#define PAGESIZE (1 << (PAGE_BITS + (H2K_KERNEL_PGSIZE * 2)))

void H2K_tcm_copy(u32_t l2_tags, u32_t last_tlb_index) {

#ifdef H2K_USE_TCM

	u32_t l2 = (H2K_gp->core_rev & CORE_REV_L2_MASK) >> CORE_REV_L2_SHIFT;
	u32_t cfg_base;
	u32_t tcm_base;
	u64_t *from, *rd;
	u64_t *to;
	u64_t *wr = NULL;  // just to keep compiler happy
	u32_t len, count;
	H2K_mem_tlbfmt_t entry;
	u32_t page;
	u32_t page_num;
	u64_t tmp;

	if (l2 * L2_CHUNK - (1 << l2_tags) * L2_TAG_CHUNK > (u32_t)&H2K_KERNEL_NPAGES * PAGESIZE) {  // room in TCM?
		
		asm volatile ( "%0 = cfgbase\n" : "=r" (cfg_base));
		tcm_base = (H2K_mem_physread_word((cfg_base << CFG_TABLE_SHIFT) + CFG_TABLE_L2TCM) << CFG_TABLE_SHIFT);

		// remap
		H2K_tmpmap_add_and_lock((pa_t)tcm_base, UC);

		for (page_num = 0; page_num < (u32_t)&H2K_KERNEL_NPAGES; page_num++) {
			entry.raw = 0;
			page = tcm_base + (page_num * PAGESIZE);
			entry.ppd = (page >> (PAGE_BITS - 1)) | (1 << H2K_KERNEL_PGSIZE);
			entry.cccc = L1WB_L2UC;
			entry.xwru = 0;  // only monitor access
			page = H2K_LINK_ADDR + (page_num * PAGESIZE);
			entry.vpn = (page >> PAGE_BITS);
			// entry.asid = don't care
			entry.global = 1;
			entry.valid = 1;

			from = (u64_t *)page;
			to = (u64_t *)(TEMP_MAP_VA + (page_num * PAGESIZE));
			len = 1 << (PAGE_BITS + (H2K_KERNEL_PGSIZE * 2));

			/* Copy a page at a time to TCM and replace the TLB entry right out from
				 under our feet.  Pretty dicey, huh? */
			asm volatile
				(
				 "   %0 = %5 \n"
				 "   %1 = %6 \n"
				 "   %2 = %7 \n"
				 "1: %3 = memd(%0) \n"
				 "   memd(%1) = %3 \n"
				 "   %0 = add(%0, #8) \n"
				 "   %1 = add(%1, #8) \n"
				 "   %2 = add(%2, #-8) \n"
				 "   p0 = cmp.eq(%2, #0) \n"
				 "   if (!p0) jump 1b \n"
				 "   tlbw(%9,%8)\n"
				 : "=&r"(rd), "=&r"(wr), "=&r"(count), "=&r"(tmp),"+m"(*wr)
				 : "r"(from), "r"(to), "r"(len), "r"(last_tlb_index + 1 + page_num),"r"(entry.raw) : "memory", "p0"
				 );
		}
		H2K_tmpmap_remove_and_unlock();

		H2K_gp->info_boot_flags.boot_use_tcm = 1;
		H2K_gp->phys_offset = H2K_LINK_ADDR - tcm_base;
	}

#endif

}

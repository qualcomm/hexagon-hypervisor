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
#include <max.h>
#include <tcm.h>
#include <cfg_table.h>

void H2K_tcm_copy(u32_t last_tlb_index) {

	if (H2K_gp->l2arr > CORE_REV_L2_CHUNK_SWITCH) {
		H2K_gp->l2size = (CORE_REV_L2_CHUNK_SWITCH * L2_CHUNK)
			+ ((H2K_gp->l2arr - CORE_REV_L2_CHUNK_SWITCH) * L2_BIG_CHUNK);
	} else {
		H2K_gp->l2size = H2K_gp->l2arr * L2_CHUNK;
	}

	H2K_gp->tcm_base = H2K_cfg_table(CFG_TABLE_L2TCM);
	H2K_gp->tcm_size = H2K_gp->l2size - (H2K_gp->l2tags > 0 ? (1 << H2K_gp->l2tags) * L2_TAG_CHUNK : 0);

#ifdef H2K_USE_TCM

	u64_t *from, *rd;
	u64_t *to;
	u64_t *wr = NULL;  // just to keep compiler happy
	u32_t len, count;
	H2K_mem_tlbfmt_t entry;
	u32_t page;  // FIXME: pa_t ?
	u32_t page_num;
	u64_t tmp;

	if (H2K_gp->tcm_size > (u32_t)&H2K_KERNEL_NPAGES * H2K_PAGESIZE) {  // room in TCM?

		// remap
		H2K_tmpmap_add_and_lock((pa_t)(H2K_gp->tcm_base), DEVICE_TYPE);

		for (page_num = 0; page_num < (u32_t)&H2K_KERNEL_NPAGES; page_num++) {
			entry.raw = 0;
			page = H2K_gp->tcm_base + (page_num * H2K_PAGESIZE);
			entry.ppd = (page >> (PAGE_BITS - 1)) | (1 << H2K_KERNEL_PGSIZE);
			entry.cccc = L1WB_L2UC;
			entry.xwru = 0;  // only monitor access
			page = H2K_LINK_ADDR + (page_num * H2K_PAGESIZE);
			entry.vpn = (page >> PAGE_BITS);
			// entry.asid = don't care
			entry.global = 1;
			entry.valid = 1;

			from = (u64_t *)page;
			to = (u64_t *)(TEMP_MAP_VA + (page_num * H2K_PAGESIZE));
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
		H2K_gp->tcm_base += (page_num * H2K_PAGESIZE);
		H2K_gp->tcm_size -= (page_num * H2K_PAGESIZE);
		H2K_gp->phys_offset = H2K_LINK_ADDR - H2K_gp->tcm_base;
	}

#endif

}

#ifdef CRASH_DEBUG

#define MAX_TCM_COPY_SIZE (512*1024)

void H2K_tcm_crash_copy()
{
	u32_t bytes;
	u32_t tcm_addr;
	u32_t tcm_size;
	int i;
	// Just in case we were using TCM crash area as STLB...
	H2K_gp->stlbptr = NULL;
	u32_t *dst = (u32_t *)(KERNEL_CRASH_TCM_ADDR); /* EJP: FIXME: we're not necessarily at the start of the page, and our pgsize isn't necessarily this big... */
	tcm_addr = H2K_gp->tcm_base;
	tcm_size = H2K_gp->tcm_size;
	if (tcm_size > MAX_TCM_COPY_SIZE) tcm_size = MAX_TCM_COPY_SIZE;
	for (i = 0; i < (tcm_size/sizeof(bytes)); i++) {
		bytes = H2K_mem_physread_word(tcm_addr+(i*sizeof(bytes)));
		dst[i] = bytes;
	}
}
#endif


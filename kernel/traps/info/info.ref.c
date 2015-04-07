/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <globals.h>
#include <max.h>
#include <physread.h>
#include <symbols.h>
#include <cfg_table.h>

u32_t H2K_trap_info(info_type op, H2K_thread_context *me) {

	u32_t val;
	u32_t l2 = H2K_gp->l2size;
	u32_t l2size;

	if (l2 > CORE_REV_L2_CHUNK_SWITCH) {
		l2size = (CORE_REV_L2_CHUNK_SWITCH * L2_CHUNK)
			+ ((l2 - CORE_REV_L2_CHUNK_SWITCH) * L2_BIG_CHUNK);
	} else {
		l2size = l2 * L2_CHUNK;
	}

	switch(op) {

	case INFO_BUILD_ID:
		return H2K_gp->build_id;

	case INFO_BOOT_FLAGS:
		return H2K_gp->info_boot_flags.raw;

	case INFO_STLB:
		return H2K_gp->info_stlb.raw;

	case INFO_SYSCFG:
		asm volatile ( "%0 = syscfg\n" : "=r" (val));
		return val;

	case INFO_REV:
		return H2K_gp->core_rev;

	case INFO_SSBASE:
		return cfg_table(CFG_TABLE_SSBASE);

	case INFO_TLB_FREE:
		return H2K_gp->last_tlb_index + 1;

	case INFO_TLB_SIZE:
		return H2K_gp->tlb_size;

	case INFO_PHYSADDR:
		return H2K_LINK_ADDR - H2K_gp->phys_offset;

	case INFO_TCM_BASE:
		return cfg_table(CFG_TABLE_L2TCM);

	case INFO_L2MEM_SIZE:
		return l2size;

	case INFO_TCM_SIZE:
		return l2size - (H2K_gp->l2_tags > 0 ? (1 << H2K_gp->l2_tags) * L2_TAG_CHUNK : 0);

	case INFO_H2K_PGSIZE:
		return H2K_PAGESIZE;

	case INFO_H2K_NPAGES:
		return (u32_t)&H2K_KERNEL_NPAGES;

	case INFO_L2VIC_BASE:
		return cfg_table(CFG_TABLE_SSBASE) + L2VIC_OFFSET;

	case INFO_TIMER_BASE:
		return cfg_table(CFG_TABLE_SSBASE) + TIMER_OFFSET;

	case INFO_TIMER_INT:
		return TIMER_INT;

	case INFO_ERROR:
		return H2K_gp->kernel_error;

	default:
		return -1;
	}
}

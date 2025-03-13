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
#include <log.h>

u32_t H2K_trap_info(info_type op, u32_t unit, h2_cfg_unit_entry entry, H2K_thread_context *me) {

	switch(op) {

	case INFO_BUILD_ID:
		return H2K_gp->build_id;

	case INFO_BOOT_FLAGS:
		return H2K_gp->info_boot_flags.raw;

	case INFO_STLB:
		return H2K_gp->info_stlb.raw;

	case INFO_SYSCFG:
		return H2K_get_syscfg();

	case INFO_LIVELOCK:
		return H2K_get_livelock();

	case INFO_REV:
		return H2K_gp->core_rev;

	case INFO_SSBASE:
		return H2K_cfg_table(CFG_TABLE_SSBASE) << CFG_TABLE_SHIFT;

	case INFO_TLB_FREE:
		return H2K_gp->last_tlb_index + 1;

	case INFO_TLB_SIZE:
		return H2K_gp->tlb_size;

	case INFO_PHYSADDR:
		return H2K_LINK_ADDR - H2K_gp->phys_offset;

	case INFO_TCM_BASE:
		return H2K_gp->tcm_base << PAGE_BITS;

	case INFO_L2MEM_SIZE:
		return H2K_gp->l2size;

	case INFO_TCM_SIZE:
		return H2K_gp->tcm_size << PAGE_BITS;

	case INFO_H2K_PGSIZE:
		return H2K_PAGESIZE;

	case INFO_H2K_NPAGES:
		return (u32_t)&H2K_KERNEL_NPAGES;

	case INFO_L2VIC_BASE:
		return (H2K_cfg_table(CFG_TABLE_SSBASE) << CFG_TABLE_SHIFT) + L2VIC_OFFSET;

	case INFO_TIMER_BASE:
		return (H2K_cfg_table(CFG_TABLE_SSBASE) << CFG_TABLE_SHIFT) + TIMER_OFFSET;

	case INFO_TIMER_INT:
		return H2K_gp->timer_intnum;

	case INFO_ERROR:
		return H2K_gp->kernel_error;

	case INFO_HTHREADS:
		return H2K_gp->hthreads_mask;

	case INFO_L2TAG_SIZE:
		return (H2K_gp->l2tags > 0 ? (1 << H2K_gp->l2tags) * L2_TAG_CHUNK : 0);

	case INFO_L2CFG_BASE:
		return H2K_cfg_table(CFG_TABLE_L2REGS) << CFG_TABLE_SHIFT;

	case INFO_CLADE_BASE:
		return H2K_cfg_table(CFG_TABLE_CLADEREGS) << CFG_TABLE_SHIFT;

	case INFO_CFGBASE:
		return H2K_get_cfgbase();

	case INFO_HVX_VLENGTH:
		return H2K_gp->hvx_vlength;

	case INFO_COPROC_CONTEXTS:
		return H2K_gp->coproc_contexts;

	case INFO_HVX_SWITCH:
#ifdef DO_EXT_SWITCH
		return me->vmblock->do_ext;
#else
		return 0;
#endif

#if ARCHV >= 65
	case INFO_VTCM_BASE:
		return H2K_gp->vtcm_base << PAGE_BITS;

	case INFO_VTCM_SIZE:
		return (H2K_gp->vtcm_size << PAGE_BITS) / 1024;

	case INFO_ECC_BASE:
		if (0x65 < H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_ECC_BASE) << CFG_TABLE_SHIFT;
		} else {
			return 0;
		}

	case INFO_L2_LINE_SZ:
		if (0x65 < H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_L2_LINE_SZ);
		} else {
			return 64;
		}

	case INFO_AUDIO_EXT:
		if (0x65 < H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_AUDIO_EXT);
		} else {
			return 0;
		}

	case INFO_VTCM_BANK_WIDTH:
		if (CORE_V68 <= H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_VTCM_BANK_WIDTH);
		} else if ((0 < H2K_gp->coproc_contexts) && (CORE_V65 <= H2K_gp->arch)) {
			return EXT_HVX_VTCM_BANK_WIDTH;
		} else {
			return 0;
		}

	case INFO_L1D_SIZE:
		if (CORE_V68 <= H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_L1D_SZ)*1024;
		} else if ((0 < H2K_gp->coproc_contexts) && (CORE_V65 <= H2K_gp->arch)) {
			return LIMIT_L1D_SZ*1024;
		} else {
			return 0;
		}
	case INFO_MAX_CLUSTER_COPROC:
#ifdef CLUSTER_SCHED
		return H2K_gp->coproc_max;
#else
		return H2K_gp->coproc_contexts;
#endif

#endif

	case INFO_HLX_CONTEXTS:
		if (CORE_V85 <= H2K_gp->arch) {
			return H2K_gp->hlx_contexts;
		} else {
			return 0;
		}

	case INFO_HMX_INSTANCES:
		return H2K_gp->hmx_units;

	case INFO_CORECFG_BASE:
		return H2K_cfg_table(CFG_TABLE_CORECFG_BASE) << CFG_TABLE_SHIFT;

	case INFO_UNIT_START:
		if (CORE_V89 <= H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_UNIT_CONFIG_REG_BASE);
		} else {
			return 0;
		}			

	case INFO_UNIT_ENTRY:
		return H2K_cfg_table_unit_entry(unit, entry);

	case INFO_CORE_ID:
		return H2K_gp->core_id;

	case INFO_CORE_COUNT:
		return H2K_gp->core_count;

	default:
		return -1;
	}
}

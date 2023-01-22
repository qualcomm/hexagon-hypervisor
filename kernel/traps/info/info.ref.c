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

u32_t H2K_trap_info(info_type op, H2K_thread_context *me) {

	//	H2K_log("Info trap %02d %s 0x%016llx boo\n", op, "and", 0xfeedbeaf12345678);

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
		return H2K_gp->tcm_base;

	case INFO_L2MEM_SIZE:
		return H2K_gp->l2size;

	case INFO_TCM_SIZE:
		return H2K_gp->tcm_size;

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

	case INFO_HVX_CONTEXTS:
		return H2K_gp->hvx_contexts;

	case INFO_HVX_SWITCH:
#ifdef DO_EXT_SWITCH
		return me->vmblock->do_ext;
#else
		return 0;
#endif

#if ARCHV >= 65
	case INFO_VTCM_BASE:
		if (0x65 < H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_VTCM_BASE) << CFG_TABLE_SHIFT;

		} else if (0 < H2K_gp->hvx_contexts && 0x65 == H2K_gp->arch) {
			return H2K_gp->tcm_base + EXT_HVX_VTCM_OFFSET;
		}
		return 0;

	case INFO_VTCM_SIZE:
		if (0x65 < H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_VTCM_SIZE);

		} else if (0 < H2K_gp->hvx_contexts && 0x65 == H2K_gp->arch) {
			return EXT_HVX_VTCM_SIZE;
		}
		return 0;

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
		} else if ((0 < H2K_gp->hvx_contexts) && (CORE_V65 <= H2K_gp->arch)) {
			return EXT_HVX_VTCM_BANK_WIDTH;
		} else {
			return 0;
		}

	case INFO_L1D_SIZE:
		if (CORE_V68 <= H2K_gp->arch) {
			return H2K_cfg_table(CFG_TABLE_L1D_SZ)*1024;
		} else if ((0 < H2K_gp->hvx_contexts) && (CORE_V65 <= H2K_gp->arch)) {
			return LIMIT_L1D_SZ*1024;
		} else {
			return 0;
		}
	case INFO_MAX_CLUSTER_COPROC:
#ifdef CLUSTER_SCHED
		return H2K_gp->coproc_max;
#else
		return H2K_gp->hvx_contexts;
#endif

#endif

	case INFO_HMX_INSTANCES:
		return H2K_gp->hmx_units;

	default:
		return -1;
	}
}

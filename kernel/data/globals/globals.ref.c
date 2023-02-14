/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <max.h>
#include <symbols.h>
#include <hw.h>
#include <bzero.h>
#include <cfg_table.h>
#include <hmx.h>

H2K_kg_t H2K_kg;

extern void _end();

void H2K_kg_init(u32_t phys_offset, u32_t devpage_offset, u32_t last_tlb_index, u32_t tlb_size) {
	u32_t l2vic_base = Q6_SS_BASE_VA + devpage_offset + L2VIC_OFFSET;
#ifdef HAVE_EXTENSIONS
	u32_t have_hvx;
	u32_t have_silver;
#endif
	
	H2K_bzero(&H2K_kg,sizeof(H2K_kg));

	asm volatile ( "%0 = rev\n" : "=r" (H2K_kg.core_rev));

	H2K_kg.phys_offset = phys_offset;
	H2K_kg.last_tlb_index = last_tlb_index;
	H2K_kg.tlb_size = tlb_size;
	H2K_kg.pinned_tlb_mask = (~0ULL) << ((last_tlb_index+1) & 0x3F);

#ifdef H2K_L2_CONTROL
	H2K_kg.l2_int_base = (void *)(l2vic_base + 0x000);
	H2K_kg.l2_ack_base = (void *)(l2vic_base + 0x200);
#endif

	H2K_kg.stlbptr = NULL;
	H2K_kg.build_id = H2K_GIT_COMMIT;
	H2K_kg.info_boot_flags.boot_use_tcm = 0;

#ifdef HAVE_EXTENSIONS
	/* HVX present? */
	if (CORE_V65 < H2K_kg.arch) {
		have_hvx = (H2K_cfg_table(CFG_TABLE_COPROC_TYPE) & CFG_TABLE_COPROC_TYPE_HVX_MASK);
		have_silver = (H2K_cfg_table(CFG_TABLE_COPROC_TYPE) & CFG_TABLE_COPROC_TYPE_SILVER_MASK);
		H2K_kg.coproc_contexts = (have_hvx || have_silver ? H2K_cfg_table(CFG_TABLE_COPROC_CONTEXTS) : 0);
#ifdef CLUSTER_SCHED
		H2K_kg.cluster_hthreads = (u32_t)(Q6_R_popcount_P(H2K_cfg_table(CFG_TABLE_HTHREADS_MASK)) / 2);
		H2K_kg.cluster_mask[0] = 0xffff >> (16 - H2K_kg.cluster_hthreads);
		H2K_kg.cluster_mask[1] = (u32_t)((0xffff >> (16 - H2K_kg.cluster_hthreads)) << H2K_kg.cluster_hthreads);
#endif	 
	} else {
		switch(H2K_kg.arch) {
		case CORE_V60:
			switch(H2K_kg.uarch) {
			case CORE_V6_A:
			case CORE_V6_E:
				have_hvx = 1;
				break;
			default:
				have_hvx = 0;
			}
			break;
		case CORE_V61:
			have_hvx = 0;
			break;
		case CORE_V62:
			switch(H2K_kg.uarch) {
			case CORE_V6_A:
			case CORE_V6_E:
				have_hvx = 1;
				break;
			default:
				have_hvx = 0;
			}
			break;
		case CORE_V65:
			switch(H2K_kg.uarch) {
			case CORE_V6_A:
				have_hvx = 1;
				break;
			default:
				have_hvx = 0;
			}
			break;
		default:
			have_hvx = 0;
		}
		H2K_kg.coproc_contexts = EXT_HVX_CONTEXTS;
		have_silver = 0;
	}
#ifdef CLUSTER_SCHED
	H2K_kg.coproc_max = H2K_kg.coproc_contexts >> 1;
#endif	
	H2K_kg.info_boot_flags.boot_have_hvx = have_hvx;
	H2K_kg.info_boot_flags.boot_have_silver = have_silver;

	if (have_hvx) {
		if (CORE_V67 < H2K_kg.arch) {
			H2K_kg.hvx_vlength = 0x1 << H2K_cfg_table(CFG_TABLE_COPROC_VLENGTH);
		} else {
			if ((H2K_kg.uarch == CORE_V6_G) || (H2K_kg.uarch == CORE_V6_Q)) {
				H2K_kg.hvx_vlength = 128;
			} else {
				H2K_kg.hvx_vlength = 64;
			}
		}
	} else {
		H2K_kg.hvx_vlength = 0;
	}

	if (CORE_V67 < H2K_kg.arch) {
#if ARCHV >= 81
		u32_t i, val;
		val = H2K_cfg_table(CFG_TABLE_HMX_INT8_RATE);
		for (i = 0; i < 4; i++) {
			H2K_kg.hmx_units += ((val & (0xff << i)) != 0);  // byte not 0?
		}
		H2K_kg.info_boot_flags.boot_have_hmx = (H2K_cfg_table(CFG_TABLE_COPROC_TYPE) & CFG_TABLE_COPROC_TYPE_HMX_MASK);
#else
		H2K_kg.hmx_units = (H2K_cfg_table(CFG_TABLE_HMX_INT8_RATE) != 0);  // exists?
		H2K_kg.info_boot_flags.boot_have_hmx = (H2K_kg.hmx_units > 0);
#endif
		H2K_kg.dma_version = H2K_cfg_table(CFG_TABLE_DMA_VERSION);
		H2K_kg.info_boot_flags.boot_have_dma = (H2K_kg.dma_version > 0);

		/* From HPG 4.8.X */
#if ARCHV >= 68
		switch (H2K_kg.arch) {
		case CORE_V68:
			H2K_kg.hmx_rsc_seq_power_on_start_addr = Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_COREV68;
			H2K_kg.hmx_rsc_seq_power_off_start_addr = Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_COREV68;
			break;
		case CORE_V69:
			H2K_kg.hmx_rsc_seq_power_on_start_addr = Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_COREV69;
			H2K_kg.hmx_rsc_seq_power_off_start_addr = Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_COREV69;
			break;
		case CORE_V73:
		default:
			H2K_kg.hmx_rsc_seq_power_on_start_addr = Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_COREV73;
			H2K_kg.hmx_rsc_seq_power_off_start_addr = Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_COREV73;
			break;
		}
#endif

	} else {
		H2K_kg.hmx_units = 0;
		H2K_kg.info_boot_flags.boot_have_hmx = 0;
		H2K_kg.dma_version = 0;
		H2K_kg.info_boot_flags.boot_have_dma = 0;
	}

	H2K_kg.info_boot_flags.boot_ext_ok = have_hvx && (!(H2K_kg.syscfg_val & SYSCFG_V2X))
		&& (H2K_kg.hthreads <= H2K_kg.coproc_contexts);

#endif

#ifdef DO_PROFILE
	H2K_kg.info_boot_flags.boot_have_sample = 1;
#else
	H2K_kg.info_boot_flags.boot_have_sample = 0;
#endif

	switch(H2K_kg.arch) {
	case CORE_V4:
		H2K_kg.timer_intnum = TIMER_INT_CORE_V4;
		break;
	case CORE_V5:
		H2K_kg.timer_intnum = TIMER_INT_CORE_V5;
		break;
	case CORE_V61:
		H2K_kg.timer_intnum = TIMER_INT_CORE_V61;
		break;
	default:
		H2K_kg.timer_intnum = TIMER_INT_CORE_V60;
		break;
	}

	H2K_kg.tcm_base = H2K_cfg_table(CFG_TABLE_L2TCM) << CFG_TABLE_SHIFT;

#if ARCHV >= 73
	// FIXME
	//	H2K_kg.dma_tlb_start = H2K_cfg_table(something);
	H2K_kg.dma_tlb_start = 512;
#endif

}

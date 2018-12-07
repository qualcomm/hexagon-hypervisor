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

H2K_kg_t H2K_kg;

void H2K_traptab();
extern u64_t H2K_stacks;

extern void _end();

void H2K_kg_init(u32_t phys_offset, u32_t devpage_offset, u32_t last_tlb_index, u32_t tlb_size) {
	u32_t l2vic_base = Q6_SS_BASE_VA + devpage_offset + L2VIC_OFFSET;
#ifdef HAVE_EXTENSIONS
	u32_t have_hvx;
#endif

	H2K_bzero(&H2K_kg,sizeof(H2K_kg));

	asm volatile ( "%0 = rev\n" : "=r" (H2K_kg.core_rev));

	H2K_kg.phys_offset = phys_offset;
	H2K_kg.last_tlb_index = last_tlb_index;
	H2K_kg.tlb_size = tlb_size;
	H2K_kg.pinned_tlb_mask = (~0ULL) << ((last_tlb_index+1) & 0x3F);
	H2K_kg.traptab_addr = H2K_traptab;
	H2K_kg.stacks_addr = &H2K_stacks;

#ifdef H2K_L2_CONTROL
	H2K_kg.l2_int_base = (void *)(l2vic_base + 0x000);
	H2K_kg.l2_ack_base = (void *)(l2vic_base + 0x200);
#endif

	H2K_kg.stlbptr = NULL;
	H2K_kg.build_id = H2K_GIT_COMMIT;
	H2K_kg.info_boot_flags.boot_use_tcm = 0;

#ifdef HAVE_EXTENSIONS
	/* HVX present? */
	if (0x65 < H2K_kg.arch) {
		have_hvx = (H2K_cfg_table(CFG_TABLE_COPROC_TYPE) == CFG_TABLE_COPROC_TYPE_HVX);
		H2K_kg.hvx_contexts = (have_hvx ? H2K_cfg_table(CFG_TABLE_COPROC_CONTEXTS) : 0);
#ifdef CLUSTER_SCHED_HACK
		H2K_kg.cluster_hthreads = (Q6_R_popcount_P(H2K_cfg_table(CFG_TABLE_HTHREADS_MASK)) / 2);
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
		H2K_kg.hvx_contexts = EXT_HVX_CONTEXTS;
	}
	H2K_kg.info_boot_flags.boot_have_hvx = have_hvx;

	if (have_hvx) {
		if (0x67 < H2K_kg.arch) {
			H2K_kg.hvx_vlength = H2K_cfg_table(CFG_TABLE_COPROC_VLENGTH);
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

	H2K_kg.hmx_units = (H2K_cfg_table(CFG_TABLE_HMX_SIZE) != 0);  // exists?

	H2K_kg.info_boot_flags.boot_ext_ok = have_hvx && (!(H2K_kg.syscfg_val & SYSCFG_V2X))
		&& (H2K_kg.hthreads <= H2K_kg.hvx_contexts);

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

}

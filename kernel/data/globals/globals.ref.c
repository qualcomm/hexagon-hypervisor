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

H2K_kg_t H2K_kg;

void H2K_traptab();
extern u64_t H2K_stacks;

extern void _end();

void H2K_kg_init(u32_t phys_offset, u32_t devpage_offset, u32_t last_tlb_index, u32_t tlb_size) {
	u32_t l2vic_base = Q6_SS_BASE_VA + devpage_offset + L2VIC_OFFSET;
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
	/* HVX present?  V6[02][AE]. */
	if ((CORE_V60 == H2K_kg.arch
			 || CORE_V62 == H2K_kg.arch
			 || CORE_V65 == H2K_kg.arch)
			&& ((CORE_V6_A == H2K_kg.uarch) || (CORE_V6_A == H2K_kg.uarch))) {
		H2K_kg.info_boot_flags.boot_have_hvx = 1;
	} else {
		H2K_kg.info_boot_flags.boot_have_hvx = 0;
	}
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

#ifdef NUM_HTHREADS
	H2K_kg.hthreads = NUM_HTHREADS;
#else
	H2K_kg.hthreads = get_hthreads();
#endif

#ifdef HTHREADS_MASK
	H2K_kg.hthreads_mask = HTHREADS_MASK;
#else
	H2K_kg.hthreads_mask = (1 << H2K_kg.hthreads) - 1;
#endif

}

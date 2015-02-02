/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <max.h>
#include <symbols.h>

H2K_kg_t H2K_kg;

void H2K_traptab();
extern u64_t H2K_stacks;

extern void _end();

void H2K_kg_init(u32_t phys_offset, u32_t devpage_offset, u32_t last_tlb_index) {
	int i;
	u64_t *x;
	u32_t l2vic_base = Q6_SS_BASE_VA + devpage_offset + L2VIC_OFFSET;

	x = (u64_t *)&H2K_kg;
	for (i = 0; i < sizeof(H2K_kg)/sizeof(*x); i++) {
		x[i] = 0;
	}

	asm volatile ( "%0 = rev\n" : "=r" (H2K_kg.core_rev));

	H2K_kg.phys_offset = phys_offset;
	H2K_kg.tlb_index = 0;
	H2K_kg.last_tlb_index = last_tlb_index;
	H2K_kg.traptab_addr = H2K_traptab;
	H2K_kg.stacks_addr = &H2K_stacks;

#ifdef H2K_L2_CONTROL
	H2K_kg.l2_int_base = (void *)(l2vic_base + 0x000);
	H2K_kg.l2_ack_base = (void *)(l2vic_base + 0x200);
#endif

	if (*(int *)((unsigned long)_end + 8) == 0x1f1f1f1f) {
		H2K_kg.on_simulator = 1;
	} else {
		H2K_kg.on_simulator = 0;
	}

	H2K_kg.stlbptr = NULL;
	H2K_kg.build_id = H2K_GIT_COMMIT;
	H2K_kg.info_boot_flags.boot_use_tcm = 0;

#ifdef HAVE_EXTENSIONS
	/* HVX present?  Only V60A, V61A. */
	if (((H2K_kg.uarch) == CORE_V60A)
			|| ((H2K_kg.uarch) == CORE_V61A)) {
		H2K_kg.info_boot_flags.boot_have_hvx = 1;
	} else {
		H2K_kg.info_boot_flags.boot_have_hvx = 0;
	}
#endif
}

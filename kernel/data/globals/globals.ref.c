/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <max.h>

H2K_kg_t H2K_kg;

void H2K_traptab();
extern u64_t H2K_stacks;

extern void _end();

void H2K_kg_init()
{
	H2K_kg.tlb_index = TLB_FIRST_REPLACEABLE_ENTRY;
	H2K_kg.traptab_addr = H2K_traptab;
	H2K_kg.stacks_addr = &H2K_stacks;
#ifdef H2K_L2_CONTROL
	/* EJP: FIXME: should read from cfg table, but it doesn't work in tools sim */
	H2K_kg.l2_int_base = (void *)(L2_INT_BASE);
	H2K_kg.l2_ack_base = (void *)(L2_INT_BASE + 0x200);

	if (*(int *)((unsigned long)_end + 8) == 0x1f1f1f1f) {
		H2K_kg.on_simulator = 1;
	} else {
		H2K_kg.on_simulator = 0;
	}

#endif
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <asid.h>
#include <hexagon_protos.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
H2K_vmblock_t vmblock;

u32_t TH_saw_tlb_invalidate;
u32_t TH_saw_stlb_invalidate;

u64_t bases[MAX_ASIDS*4];

H2K_kg_t H2K_kg;

void print_max_hops(u32_t howfull)
{
	u32_t i;
	u32_t max = 0;
	u32_t sumhops = 0;
	for (i = 0; i < MAX_ASIDS; i++) {
		max = Q6_R_maxu_RR((1<<H2K_gp->asid_table[i].fields.log_maxhops),max);
		sumhops += (1<<H2K_gp->asid_table[i].fields.log_maxhops);
	}
	printf("H2K_gp=%p addition %d: Max hops: %d avg. max hops: %f\n",H2K_gp,howfull,max,((float)sumhops)/MAX_ASIDS);
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
}

void check_max()
{
	u32_t i;
	s32_t tmp;
	H2K_asid_entry_t entry;
	vmblock.vmidx = 2;

	/* Set up KGP correctly for direct calls */
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	H2K_asid_table_init();

	for (i = 0; i < MAX_ASIDS; i++) {
		if ((tmp = H2K_asid_table_inc((u32_t)(&bases[i]), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, 0, &vmblock)) < 0) {
			FAIL("Couldn't alloc max ASIDs");
		}
		bases[i] = tmp;
		print_max_hops(i+1);
	}

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS]), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, 0, &vmblock) >= 0) {
		FAIL("Didn't fail max ASIDs+1");
	}

	for (i = 0; i < MAX_ASIDS; i++) {
		if ((tmp = H2K_asid_table_inc((u32_t)(&bases[i]), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, 0, &vmblock)) < 0) {
			FAIL("Couldn't inc max ASIDs");
		}
		bases[i] = tmp;
	}

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS]), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, 0, &vmblock) >= 0) {
		FAIL("Didn't fail max ASIDs+1");
	}

	for (i = 0; i < MAX_ASIDS; i++) {
		entry = H2K_gp->asid_table[bases[i]];
		if (entry.ptb != ((u32_t)(&bases[i]))) {
			FAIL("Didn't update PTB field correctly");
		}
		if (entry.fields.count != 2) FAIL("Didn't update counts correctly");
	}

	H2K_asid_table_dec(bases[0]);

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS]), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, 0, &vmblock) >= 0) {
		FAIL("Didn't fail max ASIDs+1 after decrement");
	}

	H2K_asid_table_dec(bases[0]);

	entry = H2K_gp->asid_table[bases[0]];
	if (entry.ptb != ((u32_t)(&bases[0]))) {
		FAIL("Didn't keep PTB field");
	}
	if (entry.fields.count != 0) FAIL("Didn't update counts correctly / dec");

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS]), H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, 0, &vmblock) < 0) {
		FAIL("couldn't alloc asid after freeing");
	}
	print_max_hops(MAX_ASIDS);
}

/* EJP: FIXME: need to check that recycling ASID calls tlb/stlb invalidation */
int main()
{
	check_max();
	puts("TEST PASSED");
	return 0;
}


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
#include <q6protos.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

u64_t bases[MAX_ASIDS*4];

extern H2K_asid_entry_t H2K_mem_asid_table[];

void print_max_hops(u32_t howfull)
{
	u32_t i;
	u32_t max = 0;
	u32_t sumhops = 0;
	for (i = 0; i < MAX_ASIDS; i++) {
		max = Q6_R_maxu_RR(H2K_mem_asid_table[i].maxhops,max);
		sumhops += H2K_mem_asid_table[i].maxhops;
	}
	printf("Max hops: %d avg. max hops: %f\n",max,((float)sumhops)/howfull);
}

void check_max()
{
	u32_t i;
	s32_t tmp;
	H2K_asid_entry_t entry;

	H2K_asid_table_init();

	for (i = 0; i < MAX_ASIDS; i++) {
		if ((tmp = H2K_asid_table_inc((u32_t)(&bases[i]))) < 0) {
			FAIL("Couldn't alloc max ASIDs");
		}
		bases[i] = tmp;
		printf("Addition %d: ",i); print_max_hops(i+1);
	}

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS])) >= 0) {
		FAIL("Didn't fail max ASIDs+1");
	}

	for (i = 0; i < MAX_ASIDS; i++) {
		if ((tmp = H2K_asid_table_inc((u32_t)(&bases[i]))) < 0) {
			FAIL("Couldn't inc max ASIDs");
		}
		bases[i] = tmp;
	}

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS])) >= 0) {
		FAIL("Didn't fail max ASIDs+1");
	}

	for (i = 0; i < MAX_ASIDS; i++) {
		entry = H2K_mem_asid_table[bases[i]];
		if (entry.ptb != ((u32_t)(&bases[i]))) {
			FAIL("Didn't update PTB field correctly");
		}
		if (entry.count != 2) FAIL("Didn't update counts correctly");
	}

	H2K_asid_table_dec(bases[0]);

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS])) >= 0) {
		FAIL("Didn't fail max ASIDs+1 after decrement");
	}

	H2K_asid_table_dec(bases[0]);

	entry = H2K_mem_asid_table[bases[0]];
	if (entry.ptb != ((u32_t)(&bases[0]))) {
		FAIL("Didn't keep PTB field");
	}
	if (entry.count != 0) FAIL("Didn't update counts correctly / dec");

	if (H2K_asid_table_inc((u32_t)(&bases[MAX_ASIDS])) < 0) {
		FAIL("couldn't alloc asid after freeing");
	}
	print_max_hops(MAX_ASIDS);
}

int main()
{
	check_max();
	puts("TEST PASSED");
	return 0;
}


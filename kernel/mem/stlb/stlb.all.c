/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stlb.h>
#include <globals.h>
#include <alloc.h>

s32_t H2K_mem_stlb_init(H2K_mem_stlb_asid_info_t *asid_info, void *bulk)
{
	H2K_gp->stlbptr = asid_info;
	int i,j;
	for (i = 0; i < MAX_ASIDS; i++) {
		for(j = 0; j < STLB_MAX_SETS/64; j++) {
			H2K_gp->stlbptr[i].valids[j] = 0;
		}
		H2K_gp->stlbptr[i].baseaddr = &((H2K_mem_tlbfmt_t *)bulk)[i * (STLB_MAX_SETS / MAX_ASIDS)];
		H2K_gp->stlbptr[i].waymask = 0xffffffff;
		H2K_gp->stlbptr[i].pagesize = 0; // FIXME?
	}
	H2K_gp->info_stlb.stlb_max_sets_log2 = STLB_MAX_SETS_LOG2;
	H2K_gp->info_stlb.stlb_max_ways = STLB_MAX_WAYS;
	H2K_gp->info_stlb.stlb_size = STLB_MULT;
	H2K_gp->info_stlb.stlb_enabled = 1;
	return STLB_ENTRIES;
}

s32_t H2K_mem_stlb_alloc() {
	void *info;
	void *bulk;
	H2K_gp->stlbptr = NULL;
	if ((info = H2K_mem_alloc(sizeof(H2K_mem_stlb_asid_info_t) * MAX_ASIDS)) == NULL) {
		return -1;
	}

	if ((bulk = H2K_mem_alloc(sizeof(H2K_mem_tlbfmt_t) * STLB_ENTRIES)) == NULL) {
		H2K_mem_alloc_free(info);
		return -1;
	}
	return H2K_mem_stlb_init((void *)info,bulk);
}

void H2K_stlb_tcmcrash_init()
{
	u32_t start_addr = KERNEL_CRASH_TCM_ADDR;
	u32_t bulk_addr = start_addr + (sizeof(H2K_mem_stlb_asid_info_t)*MAX_ASIDS);
	H2K_mem_stlb_init((void *)start_addr,(void *)bulk_addr);
}


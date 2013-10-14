/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stlb.h>
#include <globals.h>
#include <alloc.h>

s32_t H2K_mem_stlb_alloc() {

	H2K_mem_alloc_block_t block;
	int i, j;

#define STLB_ENTRIES (STLB_MAX_SETS * 2 * STLB_MAX_WAYS)

	block = H2K_mem_alloc_get(sizeof(H2K_mem_stlb_asid_info_t) * MAX_ASIDS);
	if (block.ptr == NULL) {
		H2K_gp->stlbptr = NULL;
		return -1;
	}
	H2K_gp->stlbptr = (H2K_mem_stlb_asid_info_t *)(void *)block.ptr;

	block = H2K_mem_alloc_get(sizeof(H2K_mem_tlbfmt_t) * STLB_ENTRIES);
	if (block.ptr == NULL) {
		H2K_gp->stlbptr = NULL;
		return -1;
	}

	for (i = 0; i < MAX_ASIDS; i++) {
		for(j = 0; j < STLB_MAX_SETS/64; j++) {
			H2K_gp->stlbptr[i].valids[j] = 0;
		}
		H2K_gp->stlbptr[i].baseaddr = &((H2K_mem_tlbfmt_t *)(void *)block.ptr)[i * (STLB_MAX_SETS / MAX_ASIDS)];
		H2K_gp->stlbptr[i].waymask = 0xffffffff;
		H2K_gp->stlbptr[i].pagesize = 0; // FIXME?
	}

	return STLB_ENTRIES;
}

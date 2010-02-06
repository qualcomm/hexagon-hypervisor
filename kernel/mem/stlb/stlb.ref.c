/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stlb.h>
#include <globals.h>

/* EJP: define check and match_asid */

static inline s32_t H2K_mem_stlb_find(u32_t va, u32_t asid, H2K_mem_stlb_asid_info_t *myinfo)
{
	u32_t idx = (va >> pagesize) & (MAX_SETS-1);
	int i;
	if (((myinfo->valids[idx>>6] >> (idx & 0x3f)) & 1) == 0) return -1;
	for (i = 0; i < MAX_WAYS; i++) {
		if (check(checkval,baseaddr[idx*MAX_WAYS+i])) {
			return idx*MAX_WAYS+i;
		}
	}
	return -1;
}

u64_t H2K_mem_stlb_lookup(u32_t va, u32_t asid, H2K_thread_context *me)
{
	H2K_mem_stlb_asid_info_t *myinfo;
	int i;
	if (H2K_mem_stlb_asid_infos == NULL) return 0;
	myinfo = H2K_mem_stlb_asid_infos[asid];
	if ((i = H2K_mem_stlb_find(va,asid,myinfo)) >= 0) {
		return myinfo->baseaddr[i];
	}
	return 0;
}

void H2K_mem_stlb_add(u32_t va, u32_t asid, u64_t entry, H2K_thread_context *me)
{
	H2K_mem_stlb_asid_info_t *myinfo;
	u32_t idx = (va >> pagesize) & (MAX_SETS-1);
	u32_t i;
	if (H2K_mem_stlb_asid_infos == NULL) return;
	myinfo = H2K_mem_stlb_asid_infos[asid];
	if (((myinfo->valids[idx >> 6] >> (idx & 0x3f)) & 1) == 0) {
		for (i = 0; i < MAX_WAYS; i++) {
			if (match_asid(asid,myinfo->baseaddr[idx*MAX_WAYS+i])) {
				myinfo->baseaddr[idx*MAX_WAYS+i] = 0;
			}
		}
		myinfo->valids[idx >> 6] |= (1<<(idx & 0x3f));
	}
	i = quick_random_way();
	myinfo->baseaddr[idx*MAX_WAYS+i] = entry;
}

void H2K_mem_stlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me)
{
	H2K_mem_stlb_asid_info_t *myinfo;
	int i;
	if (H2K_mem_stlb_asid_infos == NULL) return;
	myinfo = H2K_mem_stlb_asid_infos[asid];
	if ((i = H2K_mem_stlb_find(va,asid,myinfo)) >= 0) {
		myinfo->baseaddr[i] = 0;
	}
}

void H2K_mem_stlb_invalidate_asid(u32_t asid)
{
	int i;
	H2K_mem_stlb_asid_info_t *myinfo;
	if (H2K_mem_stlb_asid_infos == NULL) return;
	myinfo = H2K_mem_stlb_asid_infos[asid];
	for (i = 0; i < (MAX_SETS/64); i++) myinfo->valids[i] = 0;
	return;
}


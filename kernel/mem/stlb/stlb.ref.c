/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stlb.h>
#include <globals.h>
#include <tlbfmt.h>
#include <max.h>

/* EJP: define check and match_asid */

H2K_mem_stlb_asid_info_t *H2K_mem_stlb_asid_infos;// IN_SECTION(".data.mem.stlb"); /* MOVE TO GLOBALS */

#if __QDSP6_ARCH__ <= 3

static inline u32_t H2K_mem_stlb_check(u32_t va, u32_t asid, H2K_mem_tlbfmt_t entry)
{
	u32_t mask = 0xffffffff << (2*entry.size);
	if ((asid == entry.asid) && (((va>>PAGE_BITS) & mask) == (entry.vpn & mask)) && entry.valid) {
		return 1;
	} else {
		return 0;
	}
}

#else

static inline u32_t H2K_mem_stlb_check(u32_t va, u32_t asid, H2K_mem_tlbfmt_t tentry)
{
	u32_t asid_va = (va >> 12) | (asid << 20);
	return Q6_p_tlbmatch_PR(tentry.raw,asid_va);
}

#endif

static inline u32_t H2K_mem_stlb_match_asid(u32_t asid, H2K_mem_tlbfmt_t entry)
{
	return (asid == entry.asid);
}

static inline u32_t H2K_mem_stlb_quick_random_way()
{
	u32_t tmp;
	asm (" %0 = pcyclelo\n" :"=r"(tmp));
	return tmp & (STLB_MAX_WAYS-1);
}

static inline s32_t H2K_mem_stlb_find(u32_t va, u32_t asid, H2K_mem_stlb_asid_info_t *myinfo)
{
	u32_t idx = (va >> PAGE_BITS) & (STLB_MAX_SETS-1);
	int i;
	if (((myinfo->valids[idx>>6] >> (idx & 0x3f)) & 1) == 0) return -1;
	for (i = 0; i < STLB_MAX_WAYS; i++) {
		if (H2K_mem_stlb_check(va,asid,myinfo->baseaddr[idx*STLB_MAX_WAYS+i])) {
			return idx*STLB_MAX_WAYS+i;
		}
	}
	return -1;
}

H2K_mem_tlbfmt_t H2K_mem_stlb_lookup(u32_t va, u32_t asid, H2K_thread_context *me)
{
	H2K_mem_stlb_asid_info_t *myinfo;
	int i;
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	if (H2K_mem_stlb_asid_infos == NULL) return ret;
	myinfo = &H2K_mem_stlb_asid_infos[asid];
	if ((i = H2K_mem_stlb_find(va,asid,myinfo)) >= 0) {
		return myinfo->baseaddr[i];
	}
	return ret;
}

void H2K_mem_stlb_add(u32_t va, u32_t asid, H2K_mem_tlbfmt_t entry, H2K_thread_context *me)
{
	H2K_mem_stlb_asid_info_t *myinfo;
	u32_t idx = (va >> PAGE_BITS) & (STLB_MAX_SETS-1);
	u32_t i;
	if (H2K_mem_stlb_asid_infos == NULL) return;
	myinfo = &H2K_mem_stlb_asid_infos[asid];
	if (((myinfo->valids[idx >> 6] >> (idx & 0x3f)) & 1) == 0) {
		for (i = 0; i < STLB_MAX_WAYS; i++) {
			if (H2K_mem_stlb_match_asid(asid,myinfo->baseaddr[idx*STLB_MAX_WAYS+i])) {
				myinfo->baseaddr[idx*STLB_MAX_WAYS+i].raw = 0;
			}
		}
		myinfo->valids[idx >> 6] |= 1ULL<<(idx & 0x3f);
	}
	i = H2K_mem_stlb_quick_random_way();
	myinfo->baseaddr[idx*STLB_MAX_WAYS+i] = entry;
}

void H2K_mem_stlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me)
{
	H2K_mem_stlb_asid_info_t *myinfo;
	int i;
	if (H2K_mem_stlb_asid_infos == NULL) return;
	myinfo = &H2K_mem_stlb_asid_infos[asid];
	if ((i = H2K_mem_stlb_find(va,asid,myinfo)) >= 0) {
		myinfo->baseaddr[i].raw = 0;
	}
}

void H2K_mem_stlb_invalidate_asid(u32_t asid)
{
	int i;
	H2K_mem_stlb_asid_info_t *myinfo;
	if (H2K_mem_stlb_asid_infos == NULL) return;
	myinfo = &H2K_mem_stlb_asid_infos[asid];
	for (i = 0; i < (STLB_MAX_SETS/64); i++) myinfo->valids[i] = 0;
	return;
}

void H2K_mem_stlb_init()
{
	H2K_mem_stlb_asid_infos = NULL;
}


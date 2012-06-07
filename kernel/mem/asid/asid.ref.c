/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <asid.h>
#include <stlb.h>
#include <tlbmisc.h>
#include <q6protos.h>
#include <max.h>
#include <atomic.h>

#define HASHVAL(X) (Q6_R_extractu_RII((((unsigned int)(X)) * 2654435761UL),ASID_BITS,32-ASID_BITS))
#define NEXTIDX(X,Y) (((X)+(Y)) & ((1<<ASID_BITS)-1))

H2K_asid_entry_t H2K_mem_asid_table[MAX_ASIDS];

static inline H2K_asid_entry_t *H2K_asid_table_search(u32_t ptb)
{
	u32_t i = 0;
	u32_t idx,chain;
	u32_t maxhops;
	/* Hash ptb */
	idx = HASHVAL(ptb);
	chain = idx|1;
	/* Start search @ Hash */
	/* Search circularly */
	maxhops = H2K_mem_asid_table[idx].maxhops;
	do {
		if (H2K_mem_asid_table[idx].ptb == ptb) return H2K_mem_asid_table+idx;
		idx = NEXTIDX(idx,chain);
	} while ((++i) <= maxhops);
	/* Not found? Return NULL */
	return NULL;
}

static inline H2K_asid_entry_t *H2K_asid_table_eviction(u32_t ptb)
{
	/* Keep longest chain? */
	u32_t i = 0;
	u32_t idx,chain;
	H2K_asid_entry_t *start;
	/* Hash ptb */
	idx = HASHVAL(ptb);
	chain = idx|1;
	start = H2K_mem_asid_table+idx;
	/* Start search @ Hash */
	/* Search circularly for count==0 */
	while (H2K_atomic_compare_swap((u32_t *)&H2K_mem_asid_table[idx].count, 0, 1) != 0) {
		i++;
		idx = NEXTIDX(idx,chain);
		/* Not found? Return NULL */
		if (i >= MAX_ASIDS) return NULL;
	}
	start->maxhops = Q6_R_max_RR(start->maxhops,i);
	return H2K_mem_asid_table+idx;
}

s32_t H2K_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag)
{
	H2K_asid_entry_t *tmp;
	u32_t asid;
	
	if (type >= H2K_ASID_TRANS_TYPE_XXX_LAST) { // bad type
		return -1;
	}

	if ((tmp = H2K_asid_table_search(ptb)) != NULL) {
		H2K_atomic_add((u32_t *)&tmp->count, 1);
		asid =  tmp-H2K_mem_asid_table;

		if (flag) {
			H2K_mem_tlb_invalidate_asid(asid);
			H2K_mem_stlb_invalidate_asid(asid);
		}

		return asid;
	} else if ((tmp = H2K_asid_table_eviction(ptb)) != NULL) {
		tmp->ptb = ptb;
		tmp->transtype = type;
		asid = tmp-H2K_mem_asid_table;
		H2K_mem_tlb_invalidate_asid(asid);
		H2K_mem_stlb_invalidate_asid(asid);
		return asid;
	} else {
		return -1;
	}
}

void H2K_asid_table_dec(u32_t asid)
{
	H2K_atomic_add((u32_t *)&H2K_mem_asid_table[asid].count, -1);
}

s32_t H2K_asid_table_invalidate(u32_t ptb)
{
	H2K_asid_entry_t *tmp;
	u32_t asid;
	if ((tmp = H2K_asid_table_search(ptb)) != NULL) {
		if (tmp->count != 0) return -1;
		asid = tmp-H2K_mem_asid_table;
		H2K_mem_tlb_invalidate_asid(asid);
		H2K_mem_stlb_invalidate_asid(asid);
		tmp->ptb = 0;
		return 0;
	} else {
		/* Nothing to do! */
		return 0;
	}
}

void H2K_asid_table_init()
{
	int i;
	for (i = 0; i < MAX_ASIDS; i++) {
		H2K_mem_asid_table[i].raw = 0;
	}
}


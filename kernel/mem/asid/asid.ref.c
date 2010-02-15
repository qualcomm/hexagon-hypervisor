/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <asid.h>
#include <stlb.h>

#define HASHVAL(X) (Q6_R_extractu_RII((((unsigned int)(X)) * 2654435761UL),ASID_BITS,32-ASID_BITS))
#define NEXTIDX(X,Y) (((X)+(Y)) & ((1<<ASID_BITS)-1))

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
	maxhops = TABLE_BASE[idx].maxhops;
	do {
		if (TABLE_BASE[idx].ptb == ptb) return TABLE_BASE+idx;
		idx = NEXTIDX(idx,chain)
	} while ((i++) < maxhops);
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
	start = TABLE_BASE+idx;
	/* Start search @ Hash */
	/* Search circularly for count==0 */
	while (TABLE_BASE[idx].count != 0) {
		i++;
		idx = NEXTIDX(idx,chain);
		/* Not found? Return NULL */
		if (i >= MAX_ASIDS) return NULL;
	}
	start->maxhops = Q6_R_max_RR(start->maxhops,i);
	return TABLE_BASE+idx;
}

s32_t H2K_asid_table_inc(u32_t ptb)
{
	H2K_asid_entry_t *tmp;
	u32_t asid;
	if (tmp = H2K_asid_table_search(ptb)) {
		tmp.count++;
		return tmp-TABLE_BASE;
	} else if (tmp = H2K_asid_table_eviction(ptb)) {
		tmp.ptb = ptb;
		tmp.count = 1;
		asid = tmp-TABLE_BASE;
		H2K_mem_tlb_invalidate_asid(asid);
		H2K_mem_stlb_invalidate_asid(asid);
		return asid;
	} else {
		return -1;
	}
}

s32_t H2K_asid_table_dec(u32_t asid)
{
	TABLE_BASE[asid].count--;
}

s32_t H2K_asid_table_invalidate(u32_t ptb)
{
	H2K_asid_entry_t *tmp;
	u32_t asid;
	if (tmp = H2K_asid_table_search(ptb)) {
		asid = tmp-TABLE_BASE;
		H2K_mem_tlb_invalidate_asid(asid);
		H2K_mem_stlb_invalidate_asid(asid);
	} else {
		/* Nothing to do! */
		return 0;
	}
}


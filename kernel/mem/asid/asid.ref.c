/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <asid.h>
#include <stlb.h>
#include <tlbmisc.h>
#include <hexagon_protos.h>
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
	maxhops = H2K_mem_asid_table[idx].fields.maxhops;
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
	while (H2K_atomic_compare_swap_mask((u32_t *)&H2K_mem_asid_table[idx].fields,
																			0 << H2K_ASID_ENTRY_COUNT_POS,
																			1 << H2K_ASID_ENTRY_COUNT_POS,
																			0xffff << H2K_ASID_ENTRY_COUNT_POS) != 0) {
		i++;
		idx = NEXTIDX(idx,chain);
		/* Not found? Return NULL */
		if (i >= MAX_ASIDS) return NULL;
	}

	H2K_atomic_max_mask((u32_t *)&start->fields,
											i << H2K_ASID_ENTRY_MAXHOPS_POS,
											0xff << H2K_ASID_ENTRY_MAXHOPS_POS);
	return H2K_mem_asid_table+idx;
}

s32_t H2K_do_asid_table_inc(u32_t phys_ptb, translation_type type, tlb_invalidate_flag flag) {

	H2K_asid_entry_t *tmp;
	u32_t asid;

	if ((tmp = H2K_asid_table_search(phys_ptb)) != NULL) {
		H2K_atomic_add_mask((u32_t *)&tmp->fields,
												1 << H2K_ASID_ENTRY_COUNT_POS,
												0xffff << H2K_ASID_ENTRY_COUNT_POS);
		asid =  tmp-H2K_mem_asid_table;

		if (flag) {
			H2K_mem_tlb_invalidate_asid(asid);
			H2K_mem_stlb_invalidate_asid(asid);
		}

		return asid;
	} else if ((tmp = H2K_asid_table_eviction(phys_ptb)) != NULL) {
		tmp->ptb = phys_ptb;
		tmp->fields.transtype = type;
		asid = tmp-H2K_mem_asid_table;
		H2K_mem_tlb_invalidate_asid(asid);
		H2K_mem_stlb_invalidate_asid(asid);
		return asid;
	} else {
		return -1;
	}
}

s32_t H2K_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag, H2K_vmblock_t *vmblock)
{
	H2K_translation_t phys_translation;

	phys_translation.addr = ptb;

	if (ptb == 0) return -1; // no ptb; probably called from create and forgot to SET_PMAP_TYPE in config

	if (type == H2K_ASID_TRANS_TYPE_OFFSET) {
		return H2K_do_asid_table_inc(phys_translation.addr, type, flag);
	}
	
	if (type >= H2K_ASID_TRANS_TYPE_XXX_LAST) { // bad type
		return -1;
	}

	if (ptb == (u32_t)vmblock) { // type should have been offset
		return -1;
	}

	if (vmblock != NULL
			&& vmblock->pmap != 0  // FIXME: should be an error?
			&& ptb != vmblock->pmap) { 
		/* translate ptb using vmblock pmap */
		phys_translation = H2K_vm_translate(ptb, vmblock);
		if (!phys_translation.valid) return -1; // bad ptb
	}

	return H2K_do_asid_table_inc(phys_translation.addr, type, flag);
}

void H2K_asid_table_dec(u32_t asid)
{
	H2K_atomic_add_mask((u32_t *)&H2K_mem_asid_table[asid].fields,
											(-1 << H2K_ASID_ENTRY_COUNT_POS) & (0xffff << H2K_ASID_ENTRY_COUNT_POS),
											0xffff << H2K_ASID_ENTRY_COUNT_POS);
}

/* FIXME: Is this needed?  Not currently called; similar to tlb_invalidate_flag */
s32_t H2K_asid_table_invalidate(u32_t ptb, H2K_vmblock_t *vmblock)
{
	H2K_asid_entry_t *tmp;
	u32_t asid;
	H2K_translation_t phys_translation;

	phys_translation.addr = ptb;

	if (vmblock != NULL) { // translate ptb using vmblock pmap
		phys_translation = H2K_vm_translate(ptb, vmblock);
		if (!phys_translation.valid) return -1; // bad ptb
	}

	if ((tmp = H2K_asid_table_search(phys_translation.addr)) != NULL) {
		if (tmp->fields.count != 0) return -1;
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


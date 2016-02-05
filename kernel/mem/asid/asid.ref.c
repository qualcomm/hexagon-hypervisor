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
#include <globals.h>

#define ASID_HASHVAL(X) (Q6_R_extractu_RII((((unsigned int)(X)) * 2654435761UL),ASID_BITS,32-ASID_BITS))
#define NEXTIDX(X,Y) (((X)+(Y)) & ((1<<ASID_BITS)-1))

static inline u32_t log2_greater(u32_t x)
{
	return 32-Q6_R_cl0_R(x);
}

/*
 * EJP: FIXME: is +1 any worse than NEXTIDX?
 * EJP: FIXME: reduce maxhops storage?  If we eliminate, we'll have to search the whole table.
 * Note: can't move location since index == ASID
 * Mayb maxhops could be maxhops*4 or log(maxhops) or something.
 */
static inline H2K_asid_entry_t *H2K_asid_table_search(u32_t ptb, u32_t vmidx)
{
	u32_t i = 0;
	u32_t idx,chain;
	u32_t maxhops;
	/* Hash ptb */
	idx = ASID_HASHVAL(ptb);
	chain = idx|1;
	/* Start search @ Hash */
	/* Search circularly */
	maxhops = 1<<H2K_gp->asid_table[idx].fields.log_maxhops;
	do {
		if ((H2K_gp->asid_table[idx].ptb == ptb) && (H2K_gp->asid_table[idx].fields.vmid == vmidx))
			return H2K_gp->asid_table+idx;
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
	u32_t log_maxhops;
	H2K_asid_entry_t *start;
	/* Hash ptb */
	idx = ASID_HASHVAL(ptb);
	chain = idx|1;
	start = H2K_gp->asid_table+idx;
	/* Start search @ Hash */
	/* Search circularly for count==0 */
	while (H2K_gp->asid_table[idx].fields.count != 0) {
		i++;
		idx = NEXTIDX(idx,chain);
		/* Not found? Return NULL */
		if (i >= MAX_ASIDS) return NULL;
	}
	log_maxhops = log2_greater(i);
	start->fields.log_maxhops = max(log_maxhops,start->fields.log_maxhops);
	return H2K_gp->asid_table+idx;
}

s32_t H2K_do_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag, H2K_vmblock_t *vmblock)
{
	H2K_asid_entry_t *tmp;
	s32_t asid;
	u32_t vmidx = vmblock->vmidx;
	H2K_spinlock_lock(&H2K_gp->asid_spinlock);
	if ((tmp = H2K_asid_table_search(ptb,vmidx)) != NULL) {
		tmp->fields.count++;
		asid = tmp - H2K_gp->asid_table;
		if (flag) {
			H2K_mem_tlb_invalidate_asid(asid);
			H2K_mem_stlb_invalidate_asid(asid);
		}
	} else if ((tmp = H2K_asid_table_eviction(ptb)) != NULL) {
		tmp->ptb = ptb;
		tmp->fields.type = type;
		tmp->fields.vmid = vmidx;
		tmp->fields.count = 1;
		tmp->fields.extra = 0;
		asid = tmp - H2K_gp->asid_table;
		H2K_mem_tlb_invalidate_asid(asid);
		H2K_mem_stlb_invalidate_asid(asid);
	} else {
		asid = -1;
	}
	H2K_spinlock_unlock(&H2K_gp->asid_spinlock);
	return asid;
}

s32_t H2K_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag, H2K_vmblock_t *vmblock)
{
	if (type >= H2K_ASID_TRANS_TYPE_XXX_LAST) return -1;
	/* EJP: optimize me: could have translation-specific opportunity to pre-translate for first lookup */
	return H2K_do_asid_table_inc(ptb,type,flag,vmblock);
}

void H2K_asid_table_dec(u32_t asid)
{
	H2K_spinlock_lock(&H2K_gp->asid_spinlock);
	H2K_gp->asid_table[asid].fields.count--;
	H2K_spinlock_unlock(&H2K_gp->asid_spinlock);
}

void H2K_asid_table_init()
{
	/* FIXME: not necessary, globals already zeroed */
	int i;
	for (i = 0; i < MAX_ASIDS; i++) {
		H2K_gp->asid_table[i].raw = 0;
	}
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tlb.h>
#include <globals.h>
#include <hw.h>
#include <hexagon_protos.h>
#include <tlbmisc.h>

typedef s64_t (*tlbopptr_t)(u32_t, u32_t, u64_t, H2K_thread_context *);

static s64_t H2K_tlb_tlbread(u32_t unused0, u32_t idx, u64_t unused32, H2K_thread_context *me)
{
	return H2K_mem_tlb_read(idx);
}

static s64_t H2K_tlb_tlbwrite(u32_t unused0, u32_t idx, u64_t entry_in, H2K_thread_context *me)
{
	H2K_mem_tlbfmt_t entry;
	entry.raw = entry_in;
	entry.asid = me->ssr_asid;
	H2K_mutex_lock_tlb();
	H2K_mem_tlb_write(idx,entry.raw);
	H2K_mutex_unlock_tlb();
	return 0;
}

static s64_t H2K_tlb_tlbquery(u32_t unused0, u32_t va, u64_t unused32, H2K_thread_context *me)
{
	return H2K_mem_tlb_probe(va,me->ssr_asid);
}

static s64_t H2K_tlb_tlballoc(u32_t unused0, u32_t unused1, u64_t entry, H2K_thread_context *me)
{
	int idx,maskidx;
	u64_t mask;
	H2K_mutex_lock_tlb();
	mask = H2K_gp->pinned_tlb_mask;
	maskidx = 63-Q6_R_cl1_P(mask);
	idx = maskidx + (MAX_TLB_ENTRIES-64);
	if (mask == ~0L) {
		H2K_mutex_unlock_tlb();
		return -1;
	}
	/* We may have to shrink replaceable section */
	if (H2K_gp->last_tlb_index == idx) H2K_gp->last_tlb_index--;
	H2K_gp->pinned_tlb_mask = mask | 1ULL<<maskidx;
	H2K_mem_tlb_write(idx,entry);
	H2K_mutex_unlock_tlb();
	return idx;
}

static s64_t H2K_tlb_tlbfree(u32_t unused0, u32_t index, u64_t unused32, H2K_thread_context *me)
{
	int maskidx = index & 0x3f;
	if (index > MAX_TLB_ENTRIES) return -1;
	if (index <= H2K_gp->last_tlb_index) return -1;
	H2K_mutex_lock_tlb();
	H2K_mem_tlb_write(index,0);
	H2K_gp->pinned_tlb_mask &= ~(1ULL<<(maskidx));	/* Clear Bit */
	/* While free spots at the end, grow replaceable section */
	while ((H2K_gp->pinned_tlb_mask & (1ULL<<((H2K_gp->last_tlb_index+1) & 0x3f))) == 0ULL) {
		H2K_gp->last_tlb_index++;
	}
	H2K_mutex_unlock_tlb();
	return 0;
}

static const tlbopptr_t H2K_tlbtab[TLBOP_MAX] IN_SECTION(".data.tlb.tlbop") = {
	H2K_tlb_tlbread,
	H2K_tlb_tlbwrite,
	H2K_tlb_tlbquery,
	H2K_tlb_tlballoc,
	H2K_tlb_tlbfree,
};

s64_t H2K_tlb_tlbop(u32_t op, u32_t val32, u64_t val64, H2K_thread_context *me)
{
	if (op >= TLBOP_MAX) return -1;
	return H2K_tlbtab[op](0, val32, val64, me);
}


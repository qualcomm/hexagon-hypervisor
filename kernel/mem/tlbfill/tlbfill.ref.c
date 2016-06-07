/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <context.h>
#include <pagefault.h>
#include <globals.h>
#include <tlbfmt.h>
#include <stlb.h>
#include <linear.h>
#include <pagewalk.h>
#include <asid.h>
#include <tlbfill.h>
#include <symbols.h>
#include <atomic.h>
#include <tlbinsert.h>
#include <translate.h>

static inline void H2K_mem_tlb_insert_unlock(H2K_mem_tlbfmt_t entry, H2K_thread_context *me) {

	u32_t volatile *p_index = &H2K_gp->tlb_index;
	u32_t index;

	/* Need to tlblock even with ctlbw because futex code locks TLB */
	H2K_mutex_lock_tlb();
	index = *p_index;

	if ((index + 1) <= H2K_gp->last_tlb_index) {
		*p_index = index + 1;
	} else {
		*p_index = 0;
	}
	index &= me->tlbidxmask;
	H2K_mem_tlb_insert_index_unlock(entry, index);
}

void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me)
{
	H2K_mem_tlbfmt_t entry;
	u32_t asid = me->ssr_asid;
	H2K_translation_t trans = H2K_translate_default(va);

	if ((entry = H2K_mem_stlb_lookup(va,asid,me)).raw == 0) {
#ifdef COUNT_TLB_EVENTS
		H2K_atomic_add64(&me->vmblock->stlbmiss, 1);
#endif
	} else {
		H2K_mem_tlb_insert_unlock(entry,me);
		return;
	}
	trans = H2K_translate(trans, H2K_gp->asid_table[asid]);
	if ((entry = H2K_mem_tlbfmt_from_trans(trans,va,asid)).raw == 0) {
		H2K_mem_pagefault(va,me);
	} else {
		H2K_mem_stlb_add(va,asid,entry,me);
		H2K_mem_tlb_insert_unlock(entry,me);
	}
}

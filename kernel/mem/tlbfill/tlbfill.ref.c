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

#if ARCHV >= 4
static inline u32_t H2K_mem_tlb_v3_user_check(H2K_thread_context *me) { return 0; }
#else
static inline u32_t H2K_mem_tlb_v3_user_check(H2K_thread_context *me)
{
	return 0;
}
#endif

static inline void H2K_mem_tlb_insert_unlock(H2K_mem_tlbfmt_t entry, H2K_thread_context *me)
{
	u64_t rawentry;
	u32_t result;

#if ARCHV < 5
#ifndef USE_TLB_AUTOLOCK
	u32_t tag = entry.vpn | (entry.asid << (32 - PAGE_BITS));
#endif
#endif

	u32_t volatile *p_index = &H2K_gp->tlb_index;
	u32_t index;

#ifdef USE_TLB_AUTOLOCK
	/* Need to update the counter atomically due to TLB lock HW bug */
	u32_t old;

	while (1) {
		index = *p_index;

		if ((index + 1) < MAX_TLB_ENTRIES) {
			old = H2K_atomic_compare_swap((u32_t *)p_index, index, index + 1);
		} else {
			old = H2K_atomic_compare_swap((u32_t *)p_index, index, ((u32_t)&TLB_LAST_KERNEL_ENTRY) + 1);
		}
		if (old == index) break;
	}
#else
	/* Need to tlblock even with ctlbw because futex code needs to lock TLB */
	H2K_mutex_lock_tlb();
	index = *p_index;

	if ((index + 1) < MAX_TLB_ENTRIES) {
		*p_index = index + 1;
	} else {
		*p_index = ((u32_t)&TLB_LAST_KERNEL_ENTRY) + 1;
	}
#endif

#if ARCHV <= 3
	/* set guest bit in the ASID if this was a guest miss */
	if (me->ssr_guest) entry.guestonly = 1;
	rawentry = entry.raw;
	asm volatile
		(
		 " tlbhi = %H0\n"
		 " tlblo = %L0\n"
		 " tlbidx = %1\n" 
		 " tlbw\n"
		 " isync\n"
		 : :"r"(rawentry),"r"(index));

#else // >= V4
	rawentry = entry.raw;
	asm volatile
		(
	#if ARCHV >= 5
		 " %0 = ctlbw(%1,%2)\n"
	#else // == V4
		#ifdef USE_TLB_AUTOLOCK
		 " tlbw(%1,%2)\n"
		#else
		 " %0 = tlbp(%3)\n"
		 " p0 = tstbit(%0, #31)\n"
		 " if (!p0) jump 1f\n"
		 " tlbw(%1,%2)\n"

		#endif
	#endif
		 " isync\n"
		 "1:\n"
		 : "=&r" (result)
		 : "r"(rawentry),
			 "r"(index)
	#if ARCHV == 4
		#ifndef USE_TLB_AUTOLOCK
			 ,"r"(tag)
		 : "p0"
		#endif
	#endif
#endif
		 );

	H2K_mutex_unlock_tlb();
}

/* Walk the next table for this asid, if any, and fix up the pa in the entry.
	 FIXME: For now we just check the vmblock pmap; need to walk tables
	 recursively for multi-level translations */

static inline s32_t H2K_mem_tlb_fixup(u32_t va, u32_t ptb, H2K_mem_tlbfmt_t *entry, H2K_thread_context *me) {

	u32_t guest_addr;
	u32_t guest_size;
	u32_t guest_offset_mask;

	u32_t phys_addr;
	u32_t phys_size;
	u32_t phys_offset_mask;
	H2K_translation_t phys_translation;

	/* just leave the entry alone if no guest address bounds/translations, or if
		 the guest is using the vm pmap as its translations */
	if (me->vmblock->pmap == 0
			|| ptb == (u32_t)me->vmblock // offset mapping
			|| ptb == me->vmblock->pmap) return 0;

#if ARCHV <=3
	guest_size = entry->size;
	guest_offset_mask = (PAGE_SIZE << (guest_size * 2)) - 1;
	guest_addr = (va & guest_offset_mask);
	guest_addr |= (entry->ppn << PAGE_BITS) & ~guest_offset_mask;
#else
	guest_size = Q6_R_ct0_R(entry->ppd);
	guest_offset_mask = (PAGE_SIZE << (guest_size * 2)) - 1;
	guest_addr = (va & guest_offset_mask);
	guest_addr |= ((entry->ppd >> (guest_size + 1)) << (guest_size + PAGE_BITS)) & ~guest_offset_mask;
#endif

	phys_translation = H2K_vm_translate(guest_addr, me->vmblock);
	if (!phys_translation.valid) return -1;

	phys_addr = phys_translation.addr;

	phys_size = min(guest_size, phys_translation.size);

	phys_offset_mask = (PAGE_SIZE << (phys_size * 2)) - 1;

#if ARCHV <= 3
	entry->size = phys_size;
	u32_t xwru = phys_translation.xwru;
	entry->xwr &= (xwru >> 1);
	entry->guestonly &= ~(xwru & 1);

	entry->ppn = (phys_addr & ~phys_offset_mask) >> PAGE_BITS;
#else
	/* FIXME: How should phys_translation.cccc be merged with the guest's, if at all? */
	entry->xwru &= phys_translation.xwru;

	entry->ppd = ((phys_addr & ~phys_offset_mask) >> (PAGE_BITS - 1)) | (1 << phys_size);
#endif

	return 0;
}

void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me)
{
	H2K_mem_tlbfmt_t entry;
	u32_t asid = me->ssr_asid;
	H2K_mem_tlbfmt_t (*get_fn)(u32_t badva, H2K_thread_context *me);

	if ((entry = H2K_mem_stlb_lookup(va,asid,me)).raw != 0) {
		if (H2K_mem_tlb_v3_user_check(me)) {
#ifdef USE_TLB_AUTOLOCK
			H2K_mutex_unlock_tlb();
#endif
			return;
		}
		H2K_mem_tlb_insert_unlock(entry,me);
		return;
	}

	switch (H2K_mem_asid_table[asid].fields.transtype) {
	case H2K_ASID_TRANS_TYPE_LINEAR:
		get_fn = H2K_mem_get_linear;
		break;

	case H2K_ASID_TRANS_TYPE_TABLE:
		get_fn = H2K_mem_get_pagetable;
		break;

	case H2K_ASID_TRANS_TYPE_OFFSET:
		get_fn = H2K_vm_get_offset;
		break;

	default:
#ifdef USE_TLB_AUTOLOCK
		H2K_mutex_unlock_tlb();
#endif
		return;
	}

	if ((entry = get_fn(va,me)).raw != 0) {
		if (H2K_mem_tlb_v3_user_check(me)) {
#ifdef USE_TLB_AUTOLOCK
			H2K_mutex_unlock_tlb();
#endif
			return;
		}
		if (H2K_mem_tlb_fixup(va, H2K_mem_asid_table[asid].ptb, &entry, me) == -1) { // next translation failed
#ifdef USE_TLB_AUTOLOCK
			H2K_mutex_unlock_tlb();
#endif
			return;
		}

		H2K_mem_stlb_add(va,asid,entry,me);
		H2K_mem_tlb_insert_unlock(entry,me);
		return;
	}
	/* Unlock here in case thread is killed by pagefault */
#ifdef USE_TLB_AUTOLOCK
	H2K_mutex_unlock_tlb();
#endif
	return H2K_mem_pagefault(va,me);
}


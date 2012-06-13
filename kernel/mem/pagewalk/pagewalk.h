/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PAGEWALK_H
#define H2K_PAGEWALK_H 1

#include <c_std.h>
#include <tlbfmt.h>
#include <context.h>
#include <physread.h>

typedef union {
	u32_t raw;
	struct {
		u32_t s:3;
		u32_t rsvd:1;
		u32_t t:1;
		u32_t u:1;
		u32_t ccc:3;
		u32_t xwr:3;
		u32_t ppn:20;
	};
} H2K_pte_t;

/* This is rather complex */
/* For L1 page, look up PTB | ((va >> 22) << 2) */
/* For L2 page, we need bits (12+2*size) .. 22. */
/* We insert these (10-2*size) bits into PA starting at bit 2 */ 
static inline H2K_pte_t H2K_mem_translate_l2(u32_t va, u32_t l2addr, u32_t tablesize, u32_t pagesize)
{
	H2K_pte_t pte;
	l2addr = Q6_R_insert_RP(l2addr,(va >> (12+(2*pagesize))),Q6_P_combine_RR(tablesize*2,2));
	pte.raw = H2K_mem_physread_word(l2addr);
	/* Ignore L2 page size field & overwrite */
	pte.s = pagesize;
	return pte;
}

static inline H2K_pte_t H2K_mem_translate_l1(u32_t va, u32_t baseaddr)
{
	H2K_pte_t pte;
	u32_t size;
	pte.raw = H2K_mem_physread_word((u64_t)baseaddr | ((va>>20) & 0xffc));
	size = pte.s;
	if (size <= 4) return H2K_mem_translate_l2(va,pte.raw & -16,5-size,size);
	if (size == 7) pte.raw = 0;
	return pte;
}

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

H2K_mem_tlbfmt_t H2K_mem_translate_pagetable(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

#endif

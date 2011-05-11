/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <pagewalk.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <physread.h>
#include <hw.h>
#include <q6protos.h>
#include <context.h>
#include <asid.h>

#if __QDSP6_ARCH__ <= 3

static inline H2K_mem_tlbfmt_t H2K_pte_to_tlbfmt(H2K_pte_t pte, u32_t asid, u32_t badva)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	if ((pte.raw & 7) == 7) return ret;
	ret.ppn = pte.ppn;
	ret.ccc = pte.ccc;
	ret.xwr = (pte.xwr);
	ret.guestonly = ~(!pte.u);
	ret.asid = asid;
	ret.size = pte.s;
	ret.vpn = badva >> 12;
	ret.valid = 1;
	return ret;
}

#else

static inline H2K_mem_tlbfmt_t H2K_pte_to_tlbfmt(H2K_pte_t pte, u32_t asid, u32_t badva)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	ret.ppd = ((pte.ppn<<1) | (1<<pte.s)) & (~((1<<pte.s)-1));
	ret.cccc = pte.ccc;
	ret.xwru = (pte.xwr<<1) | pte.u;
	ret.vpn = badva >> 12;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}

#endif

/* This is rather complex */
/* For L1 page, look up PTB << 12 | ((va >> 22) << 2) */
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
	pte.raw = H2K_mem_physread_word(((u64_t)baseaddr << 12) | ((va>>20) & 0xffc));
	size = pte.s;
	if (size <= 4) return H2K_mem_translate_l2(va,pte.raw & -16,5-size,size);
	return pte;

}

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me)
{
	u32_t baseaddr;
	baseaddr = (H2K_mem_asid_table[me->ssr_asid & (MAX_ASIDS-1)].ptb);
	return H2K_mem_translate_l1(badva,baseaddr);
}

H2K_mem_tlbfmt_t H2K_mem_translate_pagetable(u32_t badva, H2K_thread_context *me)
{
	H2K_pte_t pte;
	H2K_mem_tlbfmt_t ret;
	pte = H2K_mem_pagewalk(badva,me);
	ret = H2K_pte_to_tlbfmt(pte,me->ssr_asid,badva);
	return ret;
}


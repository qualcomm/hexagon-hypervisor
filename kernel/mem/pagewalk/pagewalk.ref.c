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
	if ((pte.raw & 0x7f) == 0) return ret;
	ret.ppn = pte.ppd >> 1;
	ret.ccc = pte.cccc;
	ret.xwr = (pte.xwru >> 1);
	ret.guestonly = ~(pte.xwru & 1);
	ret.asid = asid;
	ret.size = Q6_R_ct0_R(pte.raw);
	ret.vpn = badva >> 12;
	ret.valid = 1;
	return ret;
}

#else

static inline H2K_mem_tlbfmt_t H2K_pte_to_tlbfmt(H2K_pte_t pte, u32_t asid, u32_t badva)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	ret.ppd = pte.ppd;
	ret.cccc = pte.cccc;
	ret.xwru = pte.xwru;
	ret.vpn = badva >> 12;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}

#endif

/* This is rather complex */
/* For L1 page, look up PTB << 12 | ((va >> 22) << 2) */
/* For L2 page, we need bits (12+2*size) .. 22. */
/* L2 PA starts as l2addr << 4 */
/* We insert these (10-2*size) bits into PA starting at bit 2 */ 
static inline H2K_pte_t H2K_mem_translate_l2(u32_t va, u32_t l2addr, u32_t tablesize, u32_t pagesize)
{
	u64_t shiftbuf;
	H2K_pte_t pte;
	l2addr >>= (2*tablesize);	/* We don't need the bottom 2*pagesize bits */
	va <<= 10;			/* 10 bits already translated */
	shiftbuf = (((u64_t)l2addr)<<32) | va;
	shiftbuf <<= (2*tablesize)+2+2;	/* shift in required bits + 2 (word aligned) */
	shiftbuf >>= 32;		/* Move to bottom of register */
	pte.raw = H2K_mem_physread_word(shiftbuf & (-4));
	pte.raw &= (-2 << (pagesize*2));
	pte.raw |= 1<<(pagesize*2);
	return pte;
}

static inline H2K_pte_t H2K_mem_translate_l1(u32_t va, u32_t baseaddr)
{
	H2K_pte_t pte;
	u32_t tz;
	pte.raw = H2K_mem_physread_word(((u64_t)baseaddr << 12) | ((va>>20) & 0xffc));
	tz = Q6_R_ct0_R(pte.raw);
	if (tz <= 4) return H2K_mem_translate_l2(va,pte.raw >> 1,tz,4-tz);
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


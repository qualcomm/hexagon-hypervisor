/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <pagewalk.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <hw.h>
#include <q6protos.h>
#include <context.h>
#include <asid.h>
#include <pagewalk.h>

#if __QDSP6_ARCH__ <= 3

static inline H2K_mem_tlbfmt_t H2K_pte_to_tlbfmt(H2K_pte_t pte, u32_t asid, u32_t badva)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	if ((pte.xwr) == 0) return ret;
	ret.ppn = pte.ppn;
	ret.ccc = pte.ccc;
	ret.xwr = (pte.xwr);
	ret.guestonly = ~(pte.u);
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
	if ((pte.xwr) == 0) return ret;
	ret.ppd = ((pte.ppn<<1) | (1<<pte.s)) & (~((1<<pte.s)-1));
	ret.cccc = pte.ccc;
	ret.xwru = (pte.xwr<<1) | pte.u;
	ret.vpn = badva >> 12;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}

#endif

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me)
{
	u32_t baseaddr;
	baseaddr = (H2K_mem_asid_table[me->ssr_asid & (MAX_ASIDS-1)].ptb);
	return H2K_mem_pagewalk_l1(badva,baseaddr);
}

H2K_mem_tlbfmt_t H2K_mem_get_pagetable(u32_t badva, H2K_thread_context *me)
{
	H2K_pte_t pte;
	H2K_mem_tlbfmt_t ret;
	pte = H2K_mem_pagewalk(badva,me);
	ret = H2K_pte_to_tlbfmt(pte,me->ssr_asid,badva);
	return ret;
}


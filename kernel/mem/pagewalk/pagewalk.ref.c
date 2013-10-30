/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <pagewalk.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <hw.h>
#include <hexagon_protos.h>
#include <context.h>
#include <asid.h>
#include <pagewalk.h>

#if ARCHV <= 3

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
	ret.vpn = badva >> PAGE_BITS;
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
	ret.vpn = badva >> PAGE_BITS;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}

#endif

H2K_pte_t H2K_mem_pagewalk_l1(u32_t va, u32_t baseaddr, H2K_vmblock_t *vmblock)
{
	H2K_pte_t pte;
	u32_t size;
	/* FIXME: check that effective addr is in bounds of VM */
	pte.raw = H2K_mem_physread_word((u64_t)baseaddr | ((va>>20) & 0xffc));
	size = pte.s;
	if (size <= 4) return H2K_mem_pagewalk_l2(va, pte.raw & -16, 5-size, size, vmblock);
	if (size == 7) pte.raw = 0;
	return pte;
}

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me)
{
	u32_t baseaddr;
	baseaddr = (H2K_mem_asid_table[me->ssr_asid & (MAX_ASIDS-1)].ptb);
	return H2K_mem_pagewalk_l1(badva, baseaddr, me->vmblock);
}

H2K_mem_tlbfmt_t H2K_mem_get_pagetable(u32_t badva, H2K_thread_context *me)
{
	H2K_pte_t pte;
	H2K_mem_tlbfmt_t ret;
	pte = H2K_mem_pagewalk(badva,me);
	ret = H2K_pte_to_tlbfmt(pte,me->ssr_asid,badva);
	return ret;
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBFMT_H
#define H2K_TLBFMT_H 1

#include <c_std.h>
#include <hexagon_protos.h>
#include <translate.h>
#include <h2_common_pmap.h>

typedef union {
	u64_t raw;
	struct {
		union {
			u32_t low;
			struct {
				u32_t ppd:24;
				u32_t cccc:3;
				u32_t hsv39:1;
				u32_t xwru:4;
			};
		};
		union {
			u32_t high;
			struct {
				u32_t vpn:20;
				u32_t asid:7;
#if ARCHV < 73
				u32_t abits:2;
#else
				u32_t pa3637:2;
#endif
				u32_t pa35:1;
				u32_t global:1;
				u32_t valid:1;
			};
		};
	};
} H2K_mem_tlbfmt_t;

static inline u32_t H2K_mem_tlbfmt_get_perms(H2K_mem_tlbfmt_t entry)
{
	return entry.xwru;
}

static inline u32_t H2K_mem_tlbfmt_get_size(H2K_mem_tlbfmt_t entry)
{
	return (u32_t)Q6_R_ct0_R(entry.low);
}

static inline pa_t H2K_mem_tlbfmt_get_basepa(H2K_mem_tlbfmt_t entry)
{
	pa_t ret;
	ret = entry.ppd;
	ret |= (pa_t)(entry.pa35 << (35 - PAGE_BITS + 1));  // + 1 because ppn starts at bit 1 of ppd
#if ARCHV >= 73
	ret |= (pa_t)(entry.pa3637 << (36 - PAGE_BITS + 1));
#endif
	ret &= ret - 1;	/* Clear least significant set bit */
	ret <<= (PAGE_BITS - 1);
	return ret;
}

#if ARCHV >= 73
static inline H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_trans(H2K_translation_t trans, u32_t va, u32_t asid, u32_t tlb_size_dma) {
#else
static inline H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_trans(H2K_translation_t trans, u32_t va, u32_t asid) {
#endif
	H2K_mem_tlbfmt_t ret = { .raw = 0 };
	u32_t tlbsize;
	u32_t ppd;
	// fail if no xwr or hsv39 set but there is no DMA TLB
#if ARCHV >= 73
	if ((((trans.xwru) & -2) == 0) || (trans.hsv39 && tlb_size_dma == 0)) return ret;
#else
	if (((trans.xwru) & -2) == 0) return ret;
#endif
	ret.vpn = va >> PAGE_BITS;
	ret.asid = asid;
	ret.valid = 1;
	ret.global = 0;
	ret.pa35 = (trans.pn >> (35 - PAGE_BITS)) & 0x1;
#if ARCHV < 73
	ret.abits = trans.abits;
#else
	ret.pa3637 = (trans.pn >> (36 - PAGE_BITS)) & 0x3;
#endif
	ret.xwru = trans.xwru;
	ret.cccc = trans.cccc;
	ret.hsv39 = trans.hsv39;
	tlbsize = trans.size;
	if (tlbsize > SIZE_MAX) tlbsize = PAGE_SIZE_MAX;
	ppd = trans.pn;
	ppd &= -(1<<tlbsize);
	ret.ppd = (ppd << 1) | (1<<tlbsize);
	return ret;
}

#endif


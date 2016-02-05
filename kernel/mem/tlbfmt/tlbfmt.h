/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBFMT_H
#define H2K_TLBFMT_H 1

#include <c_std.h>
#include <hexagon_protos.h>
#include <translate.h>

typedef union {
	u64_t raw;
	struct {
		union {
			u32_t low;
			struct {
				u32_t ppd:24;
				u32_t cccc:4;
				u32_t xwru:4;
			};
		};
		union {
			u32_t high;
			struct {
				u32_t vpn:20;
				u32_t asid:7;
				u32_t abits:2;
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
	return Q6_R_ct0_R(entry.low);
}

static inline pa_t H2K_mem_tlbfmt_get_basepa(H2K_mem_tlbfmt_t entry)
{
	pa_t ret;
	ret = entry.ppd;
	ret |= entry.pa35 << 24;
	ret &= ret - 1;	/* Clear least significant set bit */
	ret <<= 11;
	return ret;
}

static inline H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_trans(H2K_translation_t trans, u32_t va, u32_t asid)
{
	H2K_mem_tlbfmt_t ret = { .raw = 0 };
	u32_t tlbsize;
	u32_t ppd;
	if (((trans.xwru) & -2) == 0) return ret;
	ret.vpn = va >> 12;
	ret.asid = asid;
	ret.valid = 1;
	ret.global = 0;
	ret.pa35 = (trans.pn >> (35-PAGE_BITS)) & 1;
	ret.abits = trans.abits;
	ret.xwru = trans.xwru;
	tlbsize = trans.size >> 1;
	if (tlbsize > 6) tlbsize = 6;
	ppd = trans.pn;
	ppd &= -(1<<tlbsize);
	ret.ppd = (ppd << 1) | (1<<tlbsize);
	return ret;
}

#endif


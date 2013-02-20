/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBFMT_H
#define H2K_TLBFMT_H 1

#include <c_std.h>
#include <hexagon_protos.h>

#if ARCHV <= 3
typedef union {
	u64_t raw;
	struct {
		union {
			u32_t low;
			struct {
				u32_t ppn:20;
				u32_t size:4;
				u32_t part:2;
				u32_t ccc:3;
				u32_t xwr:3;
			};
		};
		union {
			u32_t high;
			struct {
				u32_t vpn:20;
				u32_t asid:5;
				u32_t guestonly:1;
				u32_t unused:2;
				u32_t global:1;
				u32_t valid:1;
				u32_t unused2:2;
			};
		};
	};
} H2K_mem_tlbfmt_t;

static inline u32_t H2K_mem_tlbfmt_get_perms(H2K_mem_tlbfmt_t entry)
{
	return (entry.xwr << 1) | (entry.guestonly == 0);
}

static inline u32_t H2K_mem_tlbfmt_get_size(H2K_mem_tlbfmt_t entry)
{
	return entry.size;
}

static inline pa_t H2K_mem_tlbfmt_get_basepa(H2K_mem_tlbfmt_t entry)
{
	return entry.ppn<<12;
}

#else

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
				u32_t unused:3;
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
	ret &= ret - 1;	/* Clear least significant set bit */
	ret <<= 11;
	return ret;
}

#endif

#endif


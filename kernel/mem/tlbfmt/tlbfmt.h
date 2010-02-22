/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBFMT_H
#define H2K_TLBFMT_H 1

#include <c_std.h>
#include <linear.h>
#include <pagewalk.h>

#if __QDSP6_ARCH <= 3
typedef union {
	u64_t raw;
	struct {
		union {
			u32_t low;
			struct {
				u32_t ppn:20;
				u32_t pgsize:4;
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
				u32_t guestonly:1
				u32_t unused:2;
				u32_t global:1;
				u32_t valid:1;
				u32_t unused2:2;
			};
		};
} H2K_mem_tlbfmt_t;

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

#endif

H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_linear(H2K_linear_fmt_t linear, u32_t asid);
H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_table(u32_t va, u32_t asid, pte_t pte);

#endif


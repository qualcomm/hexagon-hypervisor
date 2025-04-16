/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_LINEAR_H
#define H2_COMMON_LINEAR_H 1

#include "h2_common_c_std.h"

typedef union {
	struct {
		union {
			h2_u32_t low;
			struct {
				h2_u32_t ppn:24;
				h2_u32_t cccc:4;
				h2_u32_t xwru:4;
			};
		};
		union {
			h2_u32_t high;
			struct {
				h2_u32_t vpn:20;
				h2_u32_t size:4;
#if ARCHV < 73
				h2_u32_t abits:2;
				h2_u32_t unused:4;
#else
				h2_u32_t unused:6;
#endif
				h2_u32_t shared:1;
				h2_u32_t chain:1;
			};
		};
	};
	h2_u64_t raw;
} H2K_linear_fmt_t;

#endif

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_LINEAR_H
#define H2_COMMON_LINEAR_H 1

#include "h2_common_c_std.h"

typedef union {
	h2_u64_t raw;
	struct {
		union {
			h2_u32_t low;
			// 36-bit PA
			struct {
				h2_u32_t ppn:24;
				h2_u32_t cccc:3;
				h2_u32_t weak_ccc:1;  // old translations will set this to 0
				h2_u32_t xwru:4;
			};
			// 38-bit PA (HSV32) or 46-bit PA (HSV39)
			struct {
				h2_u32_t ppn_ext:26;
				h2_u32_t unused_ext:2;
				h2_u32_t xwru_ext:4;
			};
		};
		union {
			h2_u32_t high;
			// 36-bit PA, 32-bit VA
			struct {
				h2_u32_t vpn:20;
				h2_u32_t size:4;
#if ARCHV < 73
				h2_u32_t abits:2;
				h2_u32_t unused:3;
#else
				h2_u32_t unused:5;
#endif
				h2_u32_t extend:1;  // interpret with extensions
				h2_u32_t shared:1;  // guest -> phys should not remap this
				h2_u32_t chain:1;
			};
			// 38-bit PA (HSV32) or 46-bit PA (HSV39)
			struct {
				h2_u32_t vpn_ext:20;
				h2_u32_t size_ext:4;
				h2_u32_t cccc_ext:3;
				h2_u32_t weak_ccc_ext:1;
				h2_u32_t hsv39:1;
				h2_u32_t extend_ext:1;  // interpret with extensions
				h2_u32_t shared_ext:1;  // guest -> phys should not remap this
				h2_u32_t chain_ext:1;
			};
		};
	};
} H2K_linear_fmt_t;

#endif

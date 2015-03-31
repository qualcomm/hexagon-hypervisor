/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_LINEAR_H
#define H2_COMMON_LINEAR_H 1

#include "c_std.h"

typedef union {
	struct {
		union {
			u32_t low;
			struct {
				u32_t ppn:24;
				u32_t cccc:4;
				u32_t xwru:4;
			};
		};
		union {
			u32_t high;
			struct {
				u32_t vpn:20;
				u32_t size:4;
				u32_t abits:2;
				u32_t unused:5;
				u32_t chain:1;
			};
		};
	};
	u64_t raw;
} H2K_linear_fmt_t;

#endif

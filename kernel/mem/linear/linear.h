/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_LINEAR_H
#define H2K_LINEAR_H 1

#include <c_std.h>

u64_t H2K_mem_translate_linear(u32_t badva, H2K_thread_context *me);

typedef union {
	u64_t raw;
	struct {
		union {
			u32_t low;
			struct {
				u32_t ppd:24;
				u32_t cccc:4;
				u32_t u:1;
				u32_t xwr:3;
			};
		union {
			u32_t high;
			struct {
				u32_t vpn:20;
				u32_t asid:7;
				u32_t unused:3;
				u32_t g:1;
				u32_t v:1;
			};
		};
	};
} H2K_linear_list_t;

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
				u32_t g:1;
				u32_t v:1;
				u32_t unused2:2;
			};
		};
} H2K_v2v3_entry_t;

#endif


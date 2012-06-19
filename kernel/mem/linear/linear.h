/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_LINEAR_H
#define H2K_LINEAR_H 1

#include <c_std.h>
#include <context.h>
#include <tlbfmt.h>
#include <max.h>

typedef union {
	u64_t raw;
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
				u32_t unused:7;
				u32_t chain:1;
			};
		};
	};
} H2K_linear_fmt_t;

H2K_linear_fmt_t H2K_mem_lookup_linear(u32_t badva, u32_t list) IN_SECTION(".text.mem.linear");

H2K_mem_tlbfmt_t H2K_mem_get_linear(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.linear");

static inline u32_t H2K_mem_translate_linear(H2K_linear_fmt_t entry, u32_t va) {

	u32_t size = PAGE_SIZE << (entry.size * 2);

	return (va & (size - 1)) | (entry.ppn << PAGE_BITS);
}

#endif


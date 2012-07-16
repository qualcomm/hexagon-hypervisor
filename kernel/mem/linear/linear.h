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
#include <translate.h>

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

H2K_linear_fmt_t H2K_mem_lookup_linear(u32_t badva, u32_t list, H2K_vmblock_t *vmblock) IN_SECTION(".text.mem.linear");

H2K_mem_tlbfmt_t H2K_mem_get_linear(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.linear");

static inline H2K_translation_t H2K_mem_translate_linear(H2K_linear_fmt_t entry, u32_t va) {

	u32_t page_offset_mask = (PAGE_SIZE << (entry.size * 2)) - 1;
	H2K_translation_t ret;

	ret.raw = 0;
	ret.addr = (va & page_offset_mask) | (entry.ppn << PAGE_BITS);
	ret.size = entry.size;
	ret.cccc = entry.cccc;
	ret.xwru = entry.xwru;
	ret.valid = 1;

	return ret;
}

#endif


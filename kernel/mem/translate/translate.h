/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TRANSLATE_H
#define H2K_TRANSLATE_H 1

#include <max.h>
#include <asid_types.h>

typedef union {
	u64_t raw;
	struct {
		u32_t pn;
		u8_t size;
		u8_t xwru;
		u8_t cccc;
#if ARCHV < 73
		u8_t abits;
#else
		u8_t unused;
#endif
	};
} H2K_translation_t;

static inline H2K_translation_t H2K_translate_default(pa_t va)
{
	H2K_translation_t trans = {
		.pn = (u32_t)(va >> PAGE_BITS),
		.size = (32 - PAGE_BITS)/2,
		.xwru = 0xf,
		.cccc = 0xFF,
#if ARCHV < 73
		.abits = 0,
#else
		.unused = 0,
#endif
	};
	return trans;
}

static inline H2K_translation_t H2K_translate_bad()
{
	H2K_translation_t bad;
	bad.raw = 0ULL;
	return bad;
}

H2K_translation_t H2K_translate(H2K_translation_t in, H2K_asid_entry_t info) IN_SECTION(".text.mem.translate");

static inline pa_t H2K_translate_addr(pa_t addr_in, H2K_asid_entry_t info)
{
	H2K_translation_t translation = H2K_translate_default(addr_in);
	pa_t ret;
	translation = H2K_translate(translation, info);
	ret = translation.pn;
	ret <<= 12;
	ret |= addr_in & ((1<<PAGE_BITS)-1);
	return ret;
}

#endif

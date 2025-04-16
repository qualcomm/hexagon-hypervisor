/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <asid_types.h>
#include <vmblock.h>

#ifndef H2K_TRANSLATE_H
#define H2K_TRANSLATE_H 1

typedef union {
	u64_t raw;
	struct {
		u32_t pn;
		u32_t size:4;
		u32_t xwru:4;
		u32_t cccc:4;
		u32_t shared:1;
#if ARCHV < 73
		u32_t abits:2;
		u32_t unused:17;
#else
		u32_t unused:19;
#endif
	};
} H2K_translation_t;

static inline H2K_translation_t H2K_translate_default(pa_t va)
{
	H2K_translation_t trans = {
		.pn = (u32_t)(va >> PAGE_BITS),
		.size = (32 - PAGE_BITS)/2,
		.xwru = 0xf,
		.cccc = 0xf,
		.shared = 0,
#if ARCHV < 73
		.abits = 0,
		.unused = 0,
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

/* static inline pa_t H2K_translate_addr(pa_t addr_in, H2K_asid_entry_t info) */
/* { */
/* 	H2K_translation_t translation = H2K_translate_default(addr_in); */
/* 	pa_t ret; */
/* 	translation = H2K_translate(translation, info); */
/* 	ret = translation.pn; */
/* 	ret <<= 12; */
/* 	ret |= addr_in & ((1<<PAGE_BITS)-1); */
/* 	return ret; */
/* } */

#endif

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TRANSLATE_H
#define H2K_TRANSLATE_H 1

#include <vmblock.h>

typedef union {
	u64_t raw;
	struct {
		u32_t addr;
		struct {
			u32_t size:4;
			u32_t cccc:4;
			u32_t xwru:4;
			u32_t valid:1;
		};
	};
} H2K_translation_t;

H2K_translation_t H2K_translate(u32_t addr, H2K_vmblock_t *vmblock) IN_SECTION(".text.mem.translate");

#endif

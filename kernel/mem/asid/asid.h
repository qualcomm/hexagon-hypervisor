/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ASID_H
#define H2K_ASID_H 1

#include <c_std.h>
#include <max.h>

typedef union {
	u64_t raw;
	struct {
		u32_t ptb;
		u16_t count;
		u8_t maxhops;
		u8_t transtype;
	};
} H2K_asid_entry_t;

typedef enum {
	H2K_ASID_TRANS_TYPE_LINEAR,
	H2K_ASID_TRANS_TYPE_TABLE,
	H2K_ASID_TRANS_TYPE_XXX_LAST
} translation_type;

typedef enum {
	H2K_ASID_TLB_INVALIDATE_FALSE,
	H2K_ASID_TLB_INVALIDATE_TRUE,
	H2K_ASID_TLB_INVALIDATE_XXX_LAST
} tlb_invalidate_flag;

extern H2K_asid_entry_t H2K_mem_asid_table[] IN_SECTION(".data.mem.asid");

s32_t H2K_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag) IN_SECTION(".text.mem.asid");
void  H2K_asid_table_dec(u32_t asid) IN_SECTION(".text.mem.asid");
s32_t H2K_asid_table_invalidate(u32_t ptb) IN_SECTION(".text.mem.asid");
void  H2K_asid_table_init() IN_SECTION(".text.init.asid");

#endif


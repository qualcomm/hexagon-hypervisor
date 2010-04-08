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
		u8_t unused;
	};
} H2K_asid_entry_t;

extern H2K_asid_entry_t H2K_mem_asid_table[] IN_SECTION(".data.mem.asid");

s32_t H2K_asid_table_inc(u32_t ptb);
void  H2K_asid_table_dec(u32_t asid);
s32_t H2K_asid_table_invalidate(u32_t ptb);
void  H2K_asid_table_init();

#endif


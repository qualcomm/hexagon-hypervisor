/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ASID_H
#define H2K_ASID_H 1

#include <asid_types.h>
#include <c_std.h>
#include <max.h>
#include <vm.h>

//s32_t H2K_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag, H2K_vmblock_t *vmblock) IN_SECTION(".text.mem.asid");
s32_t H2K_asid_table_inc(u32_t ptb, translation_type type, tlb_invalidate_flag flag, u32_t extra, H2K_vmblock_t *vmblock) IN_SECTION(".text.mem.asid");

void  H2K_asid_table_dec(u32_t asid) IN_SECTION(".text.mem.asid");
void  H2K_asid_table_init() IN_SECTION(".text.init.asid");

#endif


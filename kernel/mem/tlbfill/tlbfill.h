/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBFILL_H
#define H2K_TLBFILL_H 1

void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me) IN_SECTION(".text.mem.tlb");
void H2K_mem_tlb_insert_index_unlock(H2K_mem_tlbfmt_t entry, u32_t index) IN_SECTION(".text.mem.tlb");

#endif


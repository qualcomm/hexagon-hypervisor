/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLBMISC_H
#define H2K_TLBMISC_H 1

#include <c_std.h>
void H2K_mem_tlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me);
void H2K_mem_stlb_invalidate_asid(u32_t asid, H2K_thread_context *me);

#endif


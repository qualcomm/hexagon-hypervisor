/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TLB_H
#define H2K_TLB_H 1

#include <c_std.h>
#include <context.h>
#include <h2_common_tlb.h>

s64_t H2K_tlb_tlbop(u32_t op, u32_t idx, u64_t entry, H2K_thread_context *me);

#endif

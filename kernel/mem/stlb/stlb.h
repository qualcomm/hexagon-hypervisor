/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_STLB_H
#define H2K_STLB_H 1

#include <c_std.h>
#include <max.h>
#include <tlbfmt.h>
#include <context.h>

#if (((STLB_MAX_SETS & 0xff) != 0) || ((STLB_MAX_SETS < 0xff)))
#error STLB_MAX_SETS should be power of two >256!
#endif
#if (STLB_MAX_WAYS > 32)
#error increase waymask
#endif

typedef struct {
	u64_t valids[STLB_MAX_SETS/64] __attribute__((aligned(32)));
	u32_t pagesize;
	u32_t waymask;
	H2K_mem_tlbfmt_t *baseaddr;
} H2K_mem_stlb_asid_info_t;

H2K_mem_tlbfmt_t H2K_mem_stlb_lookup(u32_t va, u32_t asid, H2K_thread_context *me) IN_SECTION(".text.mem.stlb");
void H2K_mem_stlb_add(u32_t va, u32_t asid, H2K_mem_tlbfmt_t entry, H2K_thread_context *me) IN_SECTION(".text.mem.stlb");
void H2K_mem_stlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me) IN_SECTION(".text.mem.stlb");
void H2K_mem_stlb_invalidate_asid(u32_t asid) IN_SECTION(".text.mem.stlb");
void H2K_mem_stlb_init() IN_SECTION(".text.init.stlb");

#endif


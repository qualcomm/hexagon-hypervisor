/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_STLB_H
#define H2K_STLB_H 1

#include <c_std.h>
#include <max.h>

#if ((MAX_SETS & 0xff) != 0)
#error MAX_SETS should be power of two >256!
#endif
#if (MAX_WAYS > 32)
#error increase waymask
#endif

typedef struct {
	u64_t valids[MAX_SETS/64] __attribute__((aligned(32)));
	u32_t pagesize;
	u32_t waymask;
	u64_t *baseaddr;
} H2K_mem_stlb_asid_info_t;

H2K_mem_stlb_asid_info_t **H2K_mem_stlb_asid_infos; /* MOVE TO GLOBALS */

#endif


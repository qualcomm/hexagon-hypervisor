/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PHYSREAD_H
#define H2K_PHYSREAD_H 1

#include <c_std.h>
#include <max.h>
#include <tlbfmt.h>
#include <tlbmisc.h>
#include <hw.h>
#include <hexagon_protos.h>


static inline u32_t H2K_mem_physread_word(u64_t pa)
{
	u32_t ret;
	u32_t upper_bits = (u32_t)(pa >> 11);
	u32_t lower_bits = pa & 0x7fc;
	asm (" %0 = memw_phys(%1,%2)" : "=r"(ret) :"r"(lower_bits),"r"(upper_bits));
	return ret;
}

static inline u64_t H2K_mem_physread_dword(u64_t pa)
{
	u32_t ret0,ret1;
	u32_t upper_bits = (u32_t)(pa >> 11);
	u32_t lower_bits = pa & 0x7fc;
	asm (" %0 = memw_phys(%1,%2)" : "=r"(ret0) :"r"(lower_bits),"r"(upper_bits));
	asm (" %0 = memw_phys(%1,%2)" : "=r"(ret1) :"r"(lower_bits+4),"r"(upper_bits));
	return (pa_t)Q6_P_combine_RR(ret1,ret0);
}

#endif

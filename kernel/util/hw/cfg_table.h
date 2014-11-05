/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _HEADER_CFG_TABLE_H
#define _HEADER_CFG_TABLE_H 1

#include <max.h>
#include <physread.h>

static inline u32_t cfg_table(u32_t entry) {
	u32_t val;
	asm volatile ( "%0 = cfgbase\n" : "=r" (val));
	return (H2K_mem_physread_word((val << CFG_TABLE_SHIFT) + entry) << CFG_TABLE_SHIFT);
}

#endif

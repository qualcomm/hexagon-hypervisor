/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_BOOTMAP_MACROS_H
#define H2K_BOOTMAP_MACROS_H

#include <max.h>

#define SIZE_4K 0
#define SIZE_16K 1
#define SIZE_64K 2
#define SIZE_256K 3
#define SIZE_1M 4
#define SIZE_4M 5
#define SIZE_16M 6

#define L1WB_L2UC 0
#define L1WT_L2UC 1
#define L1WB_L2UC_S 2
#define L1WT_L2UC_S 3
#define UC 4
#define L1WT_L2C 5
#define UC_S 6
#define L1WB_L2C 7
#define L1WB_L2CWB_AUX 0xa

#define MAIN 0
#define AUX 1

#define U 1
#define R 2
#define W 4
#define X 8

#define RW (R|W)
#define RX (R|X)
#define WX (W|X)
#define RWX (R|W|X)
#define UR (U|R)
#define UW (U|W)
#define UX (U|X)
#define URW (U|R|W)
#define URX (U|R|X)
#define UWX (U|W|X)
#define URWX (U|R|W|X)

#define NONE 0

/* hi word needs to be non-0; 0 marks the end of the list */
#define TLB_INVALID_ENTRY ((u64_t) 0xffffffffffffffffULL & ~(1ULL << TLB_ENTRY_VALID_BIT)),

#if ARCHV <= 3

#define MEMORY_MAP_BOOT(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
        (((u64_t)((VPN) | ((ASID) << 20) | ((G) << 28) | (1<<29)) << 32) | \
				 (u32_t)((PPN) | ((PGSIZE) << 20) | ((CFIELD) << 26) | (((PERM) >> 1) << 29) | ((MAINAUX) << 24))),

#else

#define MEMORY_MAP_BOOT(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
        (((u64_t)((VPN) | ((ASID) << 20) | ((G) << 30) | (1<<31)) << 32) | \
        (u32_t)(((PPN)<<1) | (1<<(PGSIZE)) | ((CFIELD) << 24) | ((PERM) << 28))),

#endif

#define MEMORY_MAP_THREAD(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
	{ .raw = \
		(((u64_t)((VPN) | ((PGSIZE) << 20)) << 32) | \
		(u32_t)(((PPN)) | ((CFIELD) << 24) | ((PERM) << 28))) \
	 },

#endif

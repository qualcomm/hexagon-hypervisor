/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <bootmap.h>
#include <linear.h>

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

#if __QDSP6_ARCH__ <= 3

#define U 0
#define R 1
#define W 2
#define X 4
#else

#define U 1
#define R 2
#define W 4
#define X 8
#endif

#define NONE 0

#define RW (R|W)
#define RX (R|X)
#define WX (W|X)
#define RWX (R|W|X)
#define UR (U|R)
#define UW (U|W)
#define UX (U|X)
#define URW (R|W)
#define URX (R|X)
#define UWX (W|X)
#define URWX (R|W|X)

#define MAIN 0
#define AUX 1

#if __QDSP6_ARCH__ <= 3

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
        (((u64_t)((VPN) | ((ASID) << 20) | ((G) << 28) | (1<<29)) << 32) | \
        (u32_t)((PPN) | ((PGSIZE) << 20) | ((CFIELD) << 26) | ((PERM) << 29) | ((MAINAUX) << 24))),
#else

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
        (((u64_t)((VPN) | ((ASID) << 20) | ((G) << 30) | (1<<31)) << 32) | \
        (u32_t)(((PPN)<<1) | (1<<(PGSIZE)) | ((CFIELD) << 24) | ((PERM) << 28))),

#endif

u64_t H2K_bootmap[] IN_SECTION(".data.init.bootmap") = {
#include "bootmap.def"
	0ULL,
};

#undef MEMORY_MAP

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
	{ .raw = \
		(((u64_t)((VPN) | ((PGSIZE) << 20)) << 32) | \
		(u32_t)(((PPN)) | ((CFIELD) << 24) | ((PERM) << 28))) \
	 },

H2K_linear_fmt_t H2K_linear_bootmap[] IN_SECTION(".data.init.bootmap") = {
#include "bootmap.def"
	{ .raw = 0 },
};

u64_t *H2K_bootmap_ptr = H2K_bootmap;


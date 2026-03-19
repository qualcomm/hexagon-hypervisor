/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_PMAP_H
#define H2_COMMON_PMAP_H 1

#define SIZE_DEFAULT -1
#define SIZE_4K 0
#define SIZE_16K 1
#define SIZE_64K 2
#define SIZE_256K 3
#define SIZE_1M 4
#define SIZE_4M 5
#define SIZE_16M 6
#define SIZE_64M 7
#define SIZE_256M 8
#define SIZE_1G 9

#if ARCHV < 73
#define PAGE_SIZE_MAX SIZE_16M
#else
#define PAGE_SIZE_MAX SIZE_1G
#endif

#define L1WB_L2UC 0
#define L1WT_L2UC 1
#define DEVICE_TYPE_SFC 2
#define UNCACHED_SFC 3
#define DEVICE_TYPE 4
#define L1WT_L2C 5
#define UNCACHED 6
#define L1WB_L2C 7
#define L1WB_L2CWT 8
#define L1WT_L2CWB 9
#define L1WB_L2CWB_AUX 0xa
#define L1WT_L2CWT_AUX 0xb
#define L1UC_L2CWT 0xd
#define L1UC_L2CWB 0xf

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

#define MEMORY_MAP_EXTENDED(VPN, PERM, CFIELD, PGSIZE, PPN, SHARED, HSV39) \
	{ .raw = \
			((h2_u64_t) ( ((VPN) | ((PGSIZE) << 20) | ((CFIELD) << 24) | ((HSV39) << 28) | (0x1 << 29) | ((SHARED) << 30) ) << 32) ) | \
			((h2_u32_t) (  (PPN) | ((PERM) << 28) ) )	\
	 },

#define MEMORY_MAP2(VPN, PERM, CFIELD, PGSIZE, PPN, SHARED) MEMORY_MAP_EXTENDED(VPN, PERM, CFIELD, PGSIZE, PPN, SHARED, 0)

#define MEMORY_MAP(VPN,PERM,CFIELD,PGSIZE,PPN) MEMORY_MAP2(VPN, PERM, CFIELD, PGSIZE, PPN, 0)

#endif

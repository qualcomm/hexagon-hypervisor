/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_BOOT_H
#define H2K_BOOT_H 1

#include <max.h>
#include <h2_common_pmap.h>

#if (H2K_LINK_ADDR != 0)
#define BOOT_TLB_PERM  0x0
#else
#define BOOT_TLB_PERM  0xf
#endif

// #define BOOT_TLB_PGSIZE 5  // 4M
#define BOOT_TLB_PGSIZE H2K_KERNEL_PGSIZE

//#define BOOT_TLB_ADDRBITS 12
#define BOOT_TLB_ADDRBITS H2K_KERNEL_ADDRBITS

#define BOOT_TLB_SHIFT (2 * BOOT_TLB_PGSIZE)
#define BOOT_TLB_SHIFT_4M (2 * SIZE_4M)

#define BOOT_TLB_OFFSET_BITS (BOOT_TLB_ADDRBITS + BOOT_TLB_SHIFT)
#define BOOT_TLB_PAGE_BITS (32 - BOOT_TLB_OFFSET_BITS)

#define BOOT_TLB_OFFSET_MASK ((1 << BOOT_TLB_OFFSET_BITS) - 1)
#define BOOT_TLB_PAGE_MASK (~BOOT_TLB_OFFSET_MASK)

#define BOOT_TLB_OFFSET_BITS_4M (BOOT_TLB_ADDRBITS + BOOT_TLB_SHIFT_4M)

//#define BOOT_TLB_PGNUM ((H2K_LINK_ADDR >> BOOT_TLB_OFFSET_BITS) << BOOT_TLB_SHIFT)

#define BOOT_TLBHI ((1 << (TLB_ENTRY_VALID_BIT - 32)) | (1 << (TLB_ENTRY_GLOBAL_BIT - 32)))

#define BOOT_CACHE_ATTR L1WB_L2C

#if ARCHV <= 3
#define BOOT_TLBLO ((BOOT_TLB_PGSIZE << TLB_ENTRY_SIZE_BITS) | (BOOT_CACHE_ATTR << TLB_ENTRY_C_BITS) | ((BOOT_TLB_PERM >> 1) << 29))
#define SBIT_SIZE 0
#else
#define BOOT_TLBLO ((1 << BOOT_TLB_PGSIZE) | (BOOT_CACHE_ATTR << TLB_ENTRY_C_BITS) | (BOOT_TLB_PERM << 28))
#define BOOT_TLBLO_DEV ((1 << SIZE_4M) | (UC << TLB_ENTRY_C_BITS) | (BOOT_TLB_PERM << 28))
#define SBIT_SIZE 1
#endif

#endif

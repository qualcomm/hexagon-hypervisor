/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_BOOT_H
#define H2K_BOOT_H 1

#include <max.h>

#if (H2K_LINK_ADDR != 0)
#define BOOT_TLB_PERM  0x0
#else
#define BOOT_TLB_PERM  0xf
#endif

#define BOOT_TLB_PGSIZE 5  // 4M
#define BOOT_TLB_ADDRBITS 12
#define BOOT_TLB_PGBITS (BOOT_TLB_ADDRBITS + (2*BOOT_TLB_PGSIZE))
#define BOOT_TLB_SHIFT (BOOT_TLB_PGBITS - BOOT_TLB_ADDRBITS)
#define BOOT_TLB_PGNUM ((H2K_LINK_ADDR >> BOOT_TLB_PGBITS) << BOOT_TLB_SHIFT)

#define BOOT_TLBHI ((1 << (TLB_ENTRY_VALID_BIT - 32)) | (1 << (TLB_ENTRY_GLOBAL_BIT - 32)))

#if __QDSP6_ARCH__ <= 3
#define BOOT_TLBLO ((BOOT_TLB_PGSIZE << TLB_ENTRY_SIZE_BITS) | (7 << TLB_ENTRY_C_BITS) | ((BOOT_TLB_PERM >> 1) << 29))
#define SBIT_SIZE 0
#else
#define BOOT_TLBLO ((1 << BOOT_TLB_PGSIZE) | (7 << TLB_ENTRY_C_BITS) | (BOOT_TLB_PERM << 28))
#define SBIT_SIZE 1
#endif

#endif

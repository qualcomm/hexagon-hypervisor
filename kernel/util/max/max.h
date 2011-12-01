/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_MAX_H
#define H2K_MAX_H

#if __QDSP6_ARCH__ <= 3
#define MAX_HTHREADS 6
#elif __QDSP6_ARCH__ >= 4
#define MAX_HTHREADS 3
#endif

#if __QDSP6_ARCH__ <= 3
#define MAX_TLB_ENTRIES 63
#define PHYSREAD_TEMP_MAP_IDX 63
#define PHYSREAD_TEMP_MAP_VPN 0xfffff
#else
#define MAX_TLB_ENTRIES 64
#endif

#define TLB_FIRST_REPLACEABLE_ENTRY 8

#if __QDSP6_ARCH__ <= 3
#define TLB_ENTRY_SIZE_BITS 20
#define TLB_ENTRY_C_BITS 26
#define TLB_ENTRY_GLOBAL_BIT 60
#define TLB_ENTRY_VALID_BIT 61
#else
#define TLB_ENTRY_C_BITS 24
#define TLB_ENTRY_GLOBAL_BIT 62
#define TLB_ENTRY_VALID_BIT 63
#endif

#define BOOT_TLB_PGSIZE 5  // 4M
#define BOOT_TLB_PGBITS 22
#define BOOT_TLB_ADDRBITS 12
#define BOOT_TLB_SHIFT (BOOT_TLB_PGBITS - BOOT_TLB_ADDRBITS)
#define BOOT_TLB_PGNUM ((H2K_LINK_ADDR >> BOOT_TLB_PGBITS) << BOOT_TLB_SHIFT)

#if (H2K_LINK_ADDR != 0)
#define BOOT_TLB_PERM  0x0
#else
#define BOOT_TLB_PERM  0xf
#endif

#define BOOT_TLBHI ((1 << (TLB_ENTRY_VALID_BIT - 32)) | (1 << (TLB_ENTRY_GLOBAL_BIT - 32)))
#if __QDSP6_ARCH__ <= 3
#define BOOT_TLBLO ((BOOT_TLB_PGSIZE << TLB_ENTRY_SIZE_BITS) | (7 << TLB_ENTRY_C_BITS) | ((BOOT_TLB_PERM >> 1) << 29))
#else
#define BOOT_TLBLO ((1 << BOOT_TLB_PGSIZE) | (7 << TLB_ENTRY_C_BITS) | (BOOT_TLB_PERM << 28))
#endif

#define MAX_THREADS 16

#define KERNEL_STACK_SIZE (8*31)

#define MAX_PRIOS 256
#define MAX_PRIO ((MAX_PRIOS) - 1)

#if __QDSP6_ARCH__ <= 3
#define ASID_BITS 5
#else
#define ASID_BITS 7
#endif

#define MAX_ASIDS (1<<(ASID_BITS))

#if __QDSP6_ARCH__ <= 3
#define MAX_INTERRUPTS 32
#else
#define H2K_L2_CONTROL 1
#define MAX_L2_INTERRUPTS 480
#define MAX_INTERRUPTS (32+MAX_L2_INTERRUPTS)

#ifndef L2_INT_BASE
#if __QDSP6_ARCH__ <= 4
#define L2_INT_BASE 0x28890000
#else
#define L2_INT_BASE 0x30300000
#endif
#endif
#endif

#define RESCHED_INT 3
#define VM_IPI_INT 0

#define MAX_TRACE_LEVEL 0

#define STLB_MAX_SETS 256
#define STLB_MAX_WAYS 16

#define PAGE_BITS 12 /* Minimum page size: 4K */
#define PAGE_SIZE (0x1 << PAGE_BITS)

#define SSR_IE_BIT 18

#if __QDSP6_ARCH__ <= 3
#define RESCHED_INT_INTMASK (0x80000000 >> RESCHED_INT)
#define VM_IPI_INTMASK (0x80000000 >> VM_IPI_INT)
#define SSR_GUEST_BIT 13
#define BOOT_THREAD_USR 0x00000000
#else
#define RESCHED_INT_INTMASK (0x00000001 << RESCHED_INT)
#define VM_IPI_INTMASK (0x00000001 << VM_IPI_INT)
#define SSR_GUEST_BIT 19
#define BOOT_THREAD_USR 0x00050000
#define SSR_SS_BIT 30
#endif

#define BOOT_THREAD_SSR (0x01c60000 | (1<<SSR_GUEST_BIT))
#define BOOT_THREAD_CCR 0xffff0000

/* ensure UGP is 0 for default thread ID behavior in h2 libs */
#define BOOT_THREAD_GPUGP 0

#define MAX_VM_ID 255
#define MAX_VM_CPUS 255
#define MAX_VM_INTS 65535

#define CACHEIDX_MAX 2048
#define WAYS_MAX 16
#define SETS_MAX (((32*1024)/32)/(WAYS_MAX))

#endif

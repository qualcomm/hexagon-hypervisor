/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_MAX_H
#define H2K_MAX_H

#ifndef MAX_HTHREADS
#if ARCHV <= 3
#define MAX_HTHREADS 6
#elif ARCHV == 4
#define MAX_HTHREADS 3
#elif ARCHV == 5
#define MAX_HTHREADS 6
#elif ARCHV == 6
#error probably four threads here
#endif
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

#define MAX_THREADS 16

#define KERNEL_STACK_SIZE (8*31)

#define MAX_PRIOS 256
#define MAX_PRIO ((MAX_PRIOS) - 1)
#define BEST_PRIO 0

#if __QDSP6_ARCH__ <= 3
#define ASID_BITS 5
#else
#define ASID_BITS 7
#endif

#define MAX_ASIDS (1<<(ASID_BITS))

/* EJP: FIXME: PA should be learned from cfgtable in v4+ */
#define Q6_SS_BASE_PA 0x28880000
#define Q6_SS_BASE_VA 0xFFC80000
#if ARCHV <= 4
#define TIMER_BASE_VA (Q6_SS_BASE_VA + 0x04000)
#else
#define TIMER_BASE_VA (Q6_SS_BASE_VA + 0x20000)
#endif

#define PERCPU_INTERRUPTS 32

#if __QDSP6_ARCH__ <= 3
#define MAX_INTERRUPTS 32
#else
#define H2K_L2_CONTROL 1
#define L2_CORE_INTERRUPT 31
#define MAX_L2_INTERRUPTS 480
#define MAX_INTERRUPTS (32+MAX_L2_INTERRUPTS)

#ifndef L2_INT_BASE
#define L2_INT_BASE (Q6_SS_BASE_VA + 0x10000)
#endif
#endif

#define RESCHED_INT 1
#define VM_IPI_INT 0
#if ARCHV <= 4
#define TIMER_INT 50
#else
#define TIMER_INT 35
#endif

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
#else
#define RESCHED_INT_INTMASK (0x00000001 << RESCHED_INT)
#define VM_IPI_INTMASK (0x00000001 << VM_IPI_INT)
#define SSR_GUEST_BIT 19
#define SSR_SS_BIT 30
#endif

#if __QDSP6_ARCH__ <= 4
#define BOOT_THREAD_USR 0x00000000
#elif __QDSP6_ARCH__ == 5
//HW D$ Prefetch off
//#define BOOT_THREAD_USR 0x00050000
//HW D$ Prefetch on
#define BOOT_THREAD_USR 0x00056000
#else
#endif

#define BOOT_THREAD_SSR (0x01c60000 | (1<<SSR_GUEST_BIT))
#define BOOT_THREAD_CCR 0xffff0000

/* ensure UGP is 0 for default thread ID behavior in h2 libs */
#define BOOT_THREAD_GPUGP 0

#define MAX_VM_CPUS 65535
#define MAX_VM_INTS 65535

#define CACHEIDX_MAX 2048
#define WAYS_MAX 16
#define SETS_MAX (((32*1024)/32)/(WAYS_MAX))

#define MAX_BOOT_CONTEXTS 1
#define INTS_PER_BOOT_CONTEXT 32
#define BOOT_STACK_SIZE 128

#endif

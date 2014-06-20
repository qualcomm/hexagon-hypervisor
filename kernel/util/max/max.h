/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* defines only */

#ifndef H2K_MAX_H
#define H2K_MAX_H

#include <h2_common_pmap.h>
#include <h2_common_defs.h>

#ifndef MAX_HTHREADS
#if ARCHV <= 3
#define MAX_HTHREADS 6
#elif ARCHV == 4
#define MAX_HTHREADS 3
#elif ARCHV == 5
#define MAX_HTHREADS 6
#elif ARCHV == 60
#define MAX_HTHREADS 4
#endif
#endif

//#define DO_EXT_SWITCH 1

#if ARCHV <= 3
#define PHYSREAD_TEMP_MAP_IDX 63
#define PHYSREAD_TEMP_MAP_VPN 0xfffff
#endif

#if ARCHV <= 3
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

#if ARCHV <= 3
#define ASID_BITS 5
#else
#define ASID_BITS 7
#endif

#define MAX_ASIDS (1<<(ASID_BITS))

/* QDSP6SS_PRIV_BASE_* is the subsystem base value read from cfg_table. We map that address to Q6_SS_BASE_VA. */

#define DEVICE_PAGE_SIZE SIZE_4M

#define Q6_SS_BASE_VA 0xFFC00000

#if ARCHV == 4
/* #define QDSP6SS_PRIV_BASE_FW    0x08880000 */
/* #define QDSP6SS_PRIV_BASE_SW    0x08980000 */
/* #define QDSP6SS_PRIV_BASE_LPASS 0x28880000 */

#define L2VIC_OFFSET 0x10000
#define TIMER_OFFSET 0x4000

#endif

#if ARCHV >= 5
#define QDSP6SS_PRIV_BASE_MSS   0xFC900000
#define QDSP6SS_PRIV_BASE_LPASS 0xFE280000

#define L2VIC_OFFSET 0x10000
#define TIMER_OFFSET 0x20000

#ifdef HAVE_EXTENSIONS
#define QDSP6SS_CP_CLK_CTL 0x000000fc
#define QDSP6SS_CP_CLK_CTL_DISABLE      0x00000000
#define QDSP6SS_CP_CLK_CTL_ENABLE       0x00000001

#define QDSP6SS_CP_RESET   0x000000f8
#define QDSP6SS_CP_RESET_ASSERT         0x00000001
#define QDSP6SS_CP_RESET_DEASSERT       0x00000000

#define QDSP6SS_CP_PWR_CTL 0x000000f0
#define QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON  0x011fffff
#define QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF 0x010fffff
#endif

#endif

#define PERCPU_INTERRUPTS 32

#if ARCHV <= 3
#define MAX_INTERRUPTS 32

#else
#define H2K_L2_CONTROL 1
#define L2_CORE_INTERRUPT 31
#define MAX_L2_INTERRUPTS 480
#define MAX_INTERRUPTS (32+MAX_L2_INTERRUPTS)
#endif

#define RESCHED_INT 1
#define VM_IPI_INT 0
#if ARCHV <= 4
#define TIMER_INT 50
#elif ARCHV == 5
#define TIMER_INT 34
#else
#define TIMER_INT 33
#endif

#define MAX_TRACE_LEVEL 0
#define DEFAULT_TRACE_ENTRIES 16

#define STLB_MAX_SETS_LOG2 11
#define STLB_MAX_SETS (1<<(STLB_MAX_SETS_LOG2))
#define STLB_MULT 2
#define STLB_MAX_WAYS 4
#define STLB_SHIFT 16		// optimize for 64KB pages
#define STLB_ENTRIES (STLB_MAX_SETS * STLB_MAX_WAYS * STLB_MULT)

#define PAGE_BITS 12 /* Minimum page size: 4K */
#define PAGE_SIZE (0x1 << PAGE_BITS)

#define SSR_IE_BIT 18

#if ARCHV <= 3
#define RESCHED_INT_INTMASK (0x80000000 >> RESCHED_INT)
#define VM_IPI_INTMASK (0x80000000 >> VM_IPI_INT)
#define SSR_GUEST_BIT 13
#else
#define RESCHED_INT_INTMASK (0x00000001 << RESCHED_INT)
#define VM_IPI_INTMASK (0x00000001 << VM_IPI_INT)
#define SSR_GUEST_BIT 19
#define SSR_SS_BIT 30
#endif

#ifdef HAVE_EXTENSIONS
#define SSR_XA_BITS 27
#define SSR_XA_NBITS 3
#define SSR_XA_BITS_MASK (((0x1 << (SSR_XA_NBITS)) - 1) << SSR_XA_BITS)
#define SSR_XE_BIT 31
#endif

#if ARCHV <= 4
#define BOOT_THREAD_USR 0x00000000
#elif ARCHV == 5
//HW D$ Prefetch off
//#define BOOT_THREAD_USR 0x00050000
//HW D$ Prefetch on
#define BOOT_THREAD_USR 0x00056000
#elif ARCHV == 60
//FIXME
#define BOOT_THREAD_USR 0x00056000
#else
#endif

#define BOOT_THREAD_SSR (0x01c60000 | (1<<SSR_GUEST_BIT))
#define BOOT_THREAD_CCR 0xffff0000

/* ensure UGP is 0 for default thread ID behavior in h2 libs */
#define BOOT_THREAD_GPUGP 0

#define MAX_VM_INTS 65535

#define CACHEIDX_MAX 2048
#define WAYS_MAX 16
#define SETS_MAX (((32*1024)/32)/(WAYS_MAX))

#define MAX_BOOT_CONTEXTS 1
#define INTS_PER_BOOT_CONTEXT 32
#define BOOT_STACK_SIZE 128

#define ALLOC_UNIT 8 /* Words per smallest allocatable block */
#define ALLOC_NUNITS 1024 // 32KB

/* One extra unit at the beginning to hold the tag for the first block, so that
	 pointers to allocated space are aligned.  We waste ALLOC_UNIT - 4 bytes. */
#define DEFAULT_ALLOC_HEAP_SIZE ((ALLOC_NUNITS * ALLOC_UNIT) + ALLOC_UNIT)
#endif

#define SYSCFG_M (0x1 << 0)
#define SYSCFG_I (0x1 << 1)
#define SYSCFG_D (0x1 << 2)
#define SYSCFG_T (0x1 << 3)
#define SYSCFG_G (0x1 << 4)
#define SYSCFG_R (0x1 << 5)
#define SYSCFG_C (0x1 << 6)

#ifdef HAVE_EXTENSIONS
#define SYSCFG_V2X (0x1 << 7)
#define V2X_LENGTH 7
#endif

#define SYSCFG_IDA (0x1 << 8)
#define SYSCFG_PM (0x1 << 9)
#define SYSCFG_TE (0x1 << 10)
#define SYSCFG_TL (0x1 << 11)
#define SYSCFG_KL (0x1 << 12)
#define SYSCFG_BQ (0x1 << 13)

#define SYSCFG_DMT (0x1 << 15)

#define SYSCFG_L2CFG_BITS 16
#define SYSCFG_L2CFG (0x7 << SYSCFG_L2CFG_BITS)
#define SYSCFG_L2CFG_0K (0 << SYSCFG_L2CFG_BITS)
#define SYSCFG_L2CFG_64K (1 << SYSCFG_L2CFG_BITS)
#define SYSCFG_L2CFG_128K (2 << SYSCFG_L2CFG_BITS)
#define SYSCFG_L2CFG_256K (3 << SYSCFG_L2CFG_BITS)
#define SYSCFG_L2CFG_512K (4 << SYSCFG_L2CFG_BITS)

#define SYSCFG_ITCM (0x1 << 19)

#define SYSCFG_L2NWA (0x1 << 21)
#define SYSCFG_L2NRA (0x1 << 22)

#define SYSCFG_L2WB_BIT 23
#define SYSCFG_L2WB (0x1 << SYSCFG_L2WB_BIT)
#define SYSCFG_L2P (0x1 << 24)
#define SYSCFG_L1DP (0x3 << 25)
#define SYSCFG_L1IP (0x3 << 27)
#define SYSCFG_L2PART (0x3 << 29)

#define PHYSREAD_HI_SHIFT 11

#define CFG_TABLE_SHIFT 16
#define CFG_TABLE_L2TCM 0
#define CFG_TABLE_SSBASE 8

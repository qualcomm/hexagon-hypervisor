/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* defines only */

#ifndef H2K_MAX_H
#define H2K_MAX_H

#include <h2_common_pmap.h>
#include <h2_common_defs.h>

// now in common? #define MAX_HTHREADS 8

//#define DO_EXT_SWITCH 1

#define CORE_REV_ARCH_MASK 0xff

#define CORE_REV_L2_CHUNK_SWITCH 0x8 // where we start using bigger chunks. HATE
#define L2_TAG_CHUNK 0x8000  //  32K
#define L2_CHUNK 0x20000     // 128K
#define L2_BIG_CHUNK 0x40000 // 256K

#define CORE_V4  0x04
#define CORE_V5  0x05
#define CORE_V60 0x60
#define CORE_V61 0x61
#define CORE_V62 0x62
#define CORE_V65 0x65
#define CORE_V66 0x66

#define CORE_V6_A 0x0
#define CORE_V6_E 0x4
#define CORE_V6_J 0x7
#define CORE_V6_K 0x8

#define TEMP_MAP_VA 0xff800000
#define TEMP_MAP_PG_SIZE SIZE_4M
#define TEMP_MAP_PG_MASK (0xffffffff << (PAGE_BITS + (TEMP_MAP_PG_SIZE * 2)))
#define TEMP_MAP_OFF_MASK (~TEMP_MAP_PG_MASK)

#define MAX_TLB_ENTRIES 192
#define TLB_ENTRY_C_BITS 24
#define TLB_ENTRY_GLOBAL_BIT 62
#define TLB_ENTRY_VALID_BIT 63

#define MAX_THREADS 16

#define KERNEL_STACK_SIZE (8*31)

#ifdef CRASH_DEBUG
#define KERNEL_CRASH_TCM_ADDR 0xFF070000
#endif

#define MAX_PRIOS 256
#define MAX_PRIO ((MAX_PRIOS) - 1)
#define BEST_PRIO 0

#if ARCHV <= 3
#define ASID_BITS 5
#else
#define ASID_BITS 7
#endif

#define MAX_ASIDS (1<<(ASID_BITS))

/* QDSP6SS_PRIV_BASE is the subsystem base value read from cfg_table, but we
	 map QDSP6SS_PUB_BASE to Q6_SS_BASE_VA so we can get at the public registers.
	 FIXME? Is the QDSP6SS_PUB_PRIV_OFFSET the same for all subsystems? */

#define DEVICE_PAGE_SIZE SIZE_4M

#define Q6_SS_BASE_VA 0xFFC00000
#define QDSP6SS_PUB_PRIV_OFFSET 0x80000

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
#endif

#define PERCPU_INTERRUPTS 16

#if ARCHV <= 3
#define MAX_INTERRUPTS 32

#else
#define H2K_L2_CONTROL 1
#if ARCHV >= 65
#define L2_CORE_INTERRUPT 2
#else
#define L2_CORE_INTERRUPT 31
#endif
#define MAX_L2_INTERRUPTS 480
#define MAX_INTERRUPTS (32+MAX_L2_INTERRUPTS)
#endif

#define RESCHED_INT 1
#define VM_IPI_INT 0

#define TIMER_INT_CORE_V4 50
#define TIMER_INT_CORE_V5 34
#define TIMER_INT_CORE_V60 33
#define TIMER_INT_CORE_V61 34

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
#define H2K_PAGESIZE (1 << ((PAGE_BITS) + ((H2K_KERNEL_PGSIZE) * 2)))

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

#define SSR_XA_BITS 27
#define SSR_XA_NBITS 3
#define SSR_XA_BITS_MASK (((0x1 << (SSR_XA_NBITS)) - 1) << SSR_XA_BITS)
#define SSR_XE_BIT 31

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
#define BOOT_THREAD_USR 0x00056000
#endif

#define BOOT_THREAD_SSR (0x01c60000 | (1<<SSR_GUEST_BIT))
#if ARCHV == 60
#define BOOT_THREAD_CCR 0x00130000	// for istariv1
#else
#define BOOT_THREAD_CCR 0x00170000
#endif

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

#define PHYSREAD_HI_SHIFT 11

#define CFG_TABLE_SHIFT 16
#define CFG_TABLE_L2TCM 0x0
#define CFG_TABLE_SSBASE 0x8
#define CFG_TABLE_L2REGS 0x10
#define CFG_TABLE_CLADEREGS 0x24

#define L2REGS_QOS_MODE                  0x100
#define L2REGS_QOS_MODE_TL_BIT                      7
//#define L2REGS_QOS_MODE_DEFAULT  0x0000022a

#define L2REGS_QOS_MAX_TRANS             0x104
#define L2REGS_QOS_ISSUE_RATE            0x108
#define L2REGS_QOS_DANGER_ISSUE_RATE     0x10c
#define L2REGS_QOS_SCOREBOARD_WATERMARK  0x110
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_BITS  0
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_LEN   8
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2 (((0x1 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_LEN) - 1) << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_BITS)
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_BITS  8
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_LEN   8
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO (((0x1 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_LEN) - 1) << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_BITS)
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_BITS 16
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_LEN   8
#define L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI (((0x1 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_LEN) - 1) << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_BITS)

#define L2REGS_QOS_SCOREBOARD_WATERMARK_DEFAULT ((1 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_BITS) | (24 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_BITS) | (24 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_BITS))

#define L2REGS_QOS_SYS_PRI               0x114
#define L2REGS_QOS_DANGER_0             0x1000
#define L2REGS_QOS_DANGER_1             0x2000
#define L2REGS_QOS_DANGER_2             0x3000
#define L2REGS_QOS_DANGER_3             0x4000

#define L2REGS_MAX L2REGS_QOS_DANGER_3

#define CLADEREGS_MAX                    ((CLADE_REG_PD_CHUNK * CLADE_NUM_PDS) + 0x1c)

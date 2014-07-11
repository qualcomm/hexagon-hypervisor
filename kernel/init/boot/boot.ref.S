/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>
#include <boot.h>
#include <h2_common_pmap.h>

#if ARCHV <= 3
#define DO_TLBW(HI,LO,IDX) \
	tlbidx = r##IDX ; \
	tlblo = r##LO ; \
	tlbhi = r##HI ; \
	tlbw ;
#else
#define DO_TLBW(HI,LO,IDX) \
	tlbw(R##HI##:##LO,R##IDX)
#endif

#if ARCHV <= 3
#define SETCONST(DST,VAL) \
	DST.H = # hi(VAL); \
	DST.L = # lo(VAL); 
#else
#define SETCONST(DST,VAL) \
	DST = # ## # (VAL) ;
#endif

#define MAKE_TLBHI(DST,CONSTVA,PGBITS,SHIFT) \
	SETCONST(DST,BOOT_TLBHI | (((CONSTVA) >> PGBITS)<<SHIFT))

#define MAKE_TLBLO(DST,SRCVAL,TMP) \
	SETCONST(DST,BOOT_TLBLO) ; \
	TMP = lsr(SRCVAL, #  BOOT_TLB_OFFSET_BITS); \
	DST |= asl(TMP, #  BOOT_TLB_SHIFT+SBIT_SIZE); 

#define MAKE_TLBLO_DEV(DST,SRCVAL,TMP) \
	SETCONST(DST,BOOT_TLBLO_DEV) ; \
	TMP = lsr(SRCVAL, #  BOOT_TLB_OFFSET_BITS_4M); \
	DST |= asl(TMP, #  BOOT_TLB_SHIFT_4M + SBIT_SIZE); 

FUNC_START start .entry .p2align 12
	jump 1f
	.space 0x40
1:
	 /* FIXME: Read syscfg and set bits, don't zero, in case earlier boot code
	has set it up */

	/* NOTE: 4096 bytes should contain all the way to jump to
	hyperspace... */
	r0 = #0					// 0
	syscfg = r0				// 1
	ickill					// 2
	dckill					// 3
	l2kill					// 4
	nop            // 5
	nop 					 // 6
	r0 = #(SYSCFG_I | SYSCFG_D)					// 7
	syscfg = r0				// 0
	isync					// 1 ... needs to be in same cache line as syscfg
	/* Setup EVB */
	SETCONST(r0,H2K_event_vectors)
	evb = r0

	SETCONST(r0,_SDA_BASE_)	// boot thread should do this...
	gp = r0

	SETCONST(H2K_GP,H2K_kg)

#if ARCHV == 3
	r0.h = #0x0100
	r0.l = #0x0000
	s60 = r0
#elif ARCHV == 4
	r0 = ##0x00002004
	s63 = r0
#elif ARCHV == 5
	r0 = #0     /* (New chickenbits) */
	r0 = setbit(r0,#2);	//  AXI L1 Snoop something
	r0 = setbit(r0,#27);	//  force store
	r0 = setbit(r0,#12);	//  HW loops
	s63 = r0
	r0 = #0
	r0 = setbit(r0,#22);	//  fix for address calculation corruption
	s62 = r0
#else
#endif

	/* Set up rising edge triggered */
	r0 = #-1
	/* Set imask to -1, since we are not lowest priority at boot */
 	/* (other idle threads) */
	imask = r0
	iel = r0
	iahl = r0
	cswi(r0)
	ciad(r0)

	// Read cfg_table
	r9 = cfgbase
	r9 = asl(r9, #(CFG_TABLE_SHIFT - PHYSREAD_HI_SHIFT))

	r10 = pc
	r13 = r10  // save for boot_offset

	r8 = #CFG_TABLE_SSBASE
	r14 = memw_phys(r8, r9) // r14 == SSBASE[35:16]
	r14 = asl(r14, #CFG_TABLE_SHIFT)  // SSBASE

	r15 = rev

#ifdef H2K_USE_TCM

	// First see if there's room; only possible on v5
#if ARCHV <= 4
#error USE_TCM on <= v4?
#endif

	// FIXME: We do this all over again later.  Maybe set up this part of
	// SYSCFG here.
	r16 = lsr(r15, #12)
	r16 = and(r16, #0xf)  // L2 size

	// Bail out if L2 size not 768K
	p0 = cmp.eq(r16, #6)
	if (!p0) jump .Lthere

	r8 = #CFG_TABLE_L2TCM
	r11 = memw_phys(r8, r9) // r11 == L2TCM[35:16]
	r11 = asl(r11, #CFG_TABLE_SHIFT)  // TCM base

	/* FIXME?  Go ahead and assume TCM is in lower 4G and aligned to at least
	kernel page size */

	// copy ourselves to TCM

	// r10 == pc
	r10 = and(r10, ##BOOT_TLB_PAGE_MASK)  // first kernel page
	SETCONST(r6, H2K_LINK_ADDR)
	SETCONST(r7, end)
	r7 = sub(r7, r6)  // size
	r6 = r11
	r5 = #0	
3:
	r1:0 = memd(r10)
	memd(r6) = r1:0
	r10 = add(r10, #8)
	r6 = add(r6, #8)
	r5 = add(r5, #8)
	p0 = cmp.gt(r5, r7)
	if (!p0) jump 3b

	// go there
	SETCONST(r10, .Lthere)
	r10 = and(r10, ##BOOT_TLB_OFFSET_MASK)  // offset of .Lthere
	r7 = or(r11, r10)
	jumpr r7

.Lthere:

#endif

	r16 = and(r15, #0xff)    // arch version
	p0 = cmp.eq(r16, #0x60)  // v60?
	if (p0) jump .Lv60

	// < v60
	r16 = lsr(r15, #8)
	r16 = and(r16, #0xf)  // uarch config
	p0 = cmp.eq(r16, #2)  // v55c
	if (p0) jump .Ljtlb64
	p0 = cmp.eq(r16, #3)  // v55d
	if (p0) jump .Ljtlb64

	r16 = #128            // all others
	jump .Ltlbsetup

.Lv60:
	r16 = lsr(r15, #8)
	r16 = and(r16, #0xf)  // uarch config
	p0 = cmp.eq(r16, #2)  // v60c
	if (p0) jump .Ljtlb64
	p0 = cmp.eq(r16, #5)  // v60f
	if (p0) jump .Ljtlb64

	r16 = #128            // all others
	jump .Ltlbsetup

.Ljtlb64:
	r16 = #64

.Ltlbsetup:
	/* Clear TLB */
	r3:2 = #0
	r1 = #0
4:
	DO_TLBW(3,2,1)

	r1 = add(r1,#1)
	p0 = cmp.eq(r1, r16)  // # tlb entries
	if (!p0) jump 4b

	// Set up TLB entries for jump to virtual space
	r10 = pc

	SETCONST(r4, H2K_KERNEL_NPAGES)
	r1 = sub(r16, r4)
	r1 = add(r1, #-1)  // starting index for pinned entries
	r16 = add(r1, #-1) // last free entry
	r5 = #0

2:
	r2 = asl(r5, #BOOT_TLB_OFFSET_BITS)
	SETCONST(r3, H2K_LINK_ADDR)
	r3 = add(r3, r2)
	r3 = lsr(r3, #BOOT_TLB_OFFSET_BITS)
	r3 = asl(r3, #BOOT_TLB_SHIFT)
	SETCONST(r2, BOOT_TLBHI)
	r3 = or(r3, r2)
	MAKE_TLBLO(r2,r10,r11)
	DO_TLBW(3,2,1)

	r1 = add(r1, #1)
	r5 = add(r5, #1)
	r10 = pc
	r10 += asl(r1,#BOOT_TLB_OFFSET_BITS)
	r4 = add(r4, #-1)
	p0 = cmp.eq(r4, #0)
	if (!p0) jump 2b

#if ARCHV > 3
	// Set up entry for L2 interrupt controller
	MAKE_TLBHI(r3,Q6_SS_BASE_VA,BOOT_TLB_OFFSET_BITS_4M,BOOT_TLB_SHIFT_4M)
	// r14 == SSBASE (PRIV_BASE)
	// apply offet to get PUB_BASE
	r14 = add(r14, ##-QDSP6SS_PUB_PRIV_OFFSET)
	MAKE_TLBLO_DEV(r2, r14, r11)
	DO_TLBW(3,2,1)

	// index for temporary entry
	r1 = #0
#endif

	// Don't make a temporary entry if we're loaded at the link address,
	// to avoid multiple tlb match since the permanent entry will match.
	r10 = pc
	SETCONST(r11,H2K_LINK_ADDR)
	r12 = and(r11,r10)
	p0 = cmp.eq(r12,r11)
	if (p0) jump 1f

	/* This one page had better be enough */
	MAKE_TLBHI(r3,0,BOOT_TLB_OFFSET_BITS,BOOT_TLB_SHIFT)
	r3 |= lsr(r10,#BOOT_TLB_ADDRBITS)
	MAKE_TLBLO(r2,r10,r11)
	DO_TLBW(3,2,1)
1:
	/* Turn on translation, ISDB, PMU, ignore data aborts, TLB lock, etc */

#ifdef ISDBEN
	r0 = #1;
	isdben = r0
#endif

#define SYSCFG_LO_ALL ( \
				  SYSCFG_M \
				| SYSCFG_I \
				| SYSCFG_D \
				| SYSCFG_T \
				| SYSCFG_G \
				| SYSCFG_R \
				| SYSCFG_C \
				| SYSCFG_IDA \
				| SYSCFG_PM \
				)

#if ARCHV <= 4
 	r0 = #SYSCFG_LO_ALL
#else
	r0 = ##(SYSCFG_LO_ALL | SYSCFG_BQ | SYSCFG_DMT)
#endif

#define SYSCFG_HI_ALL (SYSCFG_L2WB)

#if ARCHV <= 3
	r0.h = #((SYSCFG_HI_ALL | SYSCFG_L2CFG_256K) >> 16)
	//r0.h = #0x0083	/* 256KB L2 */
#elif ARCHV == 4
	r0.h = #((SYSCFG_HI_ALL | SYSCFG_ITCM) >> 16)
	r2 = ##SYSCFG_L2CFG_256K
	r15 = rev
	r15 = lsr(r15, #8)
	r15 = and(r15, #0xff)
	p0 = cmp.eq(r15, #0)
	if (!p0) jump 2f
	r2 = ##SYSCFG_L2CFG_512K
2:
	r0 = or(r0, r2)
#else // ARCHV >= 5
	r0.h = #((SYSCFG_HI_ALL | SYSCFG_ITCM) >> 16)

	r15 = rev
	r15 = extractu (r15, #4, #12)  // L2 size from rev

// get table address; r10 == phys pc
	r10 = and(r10, ##BOOT_TLB_PAGE_MASK)  // phys page

	SETCONST(r11, l2_sizes)
	r11 = and(r11, ##BOOT_TLB_OFFSET_MASK)  // offset
	r11 = or(r10, r11)
	r11 = add(r11, r15)  // entry pointer

	r15 = memb(r11)  // value
	r0 = insert(r15, #3, #16)

#endif

	syscfg = r0
	isync
	// make the jump to hyperspace
	r2.h = #hi(1f)
	r2.l = #lo(1f)
	jumpr r2

1:
	// invalidate temporary TLB entry
	r3:2 = #0
	DO_TLBW(3,2,1)
	isync

	SETCONST(r29,H2K_stacks)
	// lock kernel

	k0lock

	// Wake up other threads
	r0 = #(1<<MAX_HTHREADS)-1
	start(r0)

	// r10 == physical page after possible copy to TCM; r13 == phys pc at load addr
	SETCONST(r11, H2K_LINK_ADDR)

	r0 = sub(r11, r10)  // phys_offset

	r13 = and(r13, ##BOOT_TLB_PAGE_MASK)
	r1 = sub(r11, r13)  // boot_offset

	r2 = r14  // ssbase
	r3 = r16  // # last_tlb_index

	jump H2K_thread_boot
FUNC_END start

FUNC_START h2_init .text.init.boot .p2align 2
	jumpr r31
FUNC_END h2_init

FUNC_START H2K_handle_reset .text.init.boot .p2align 2
	SETCONST(r0,_SDA_BASE_)
	gp = r0

	SETCONST(H2K_GP,H2K_kg)

	SETCONST(r29,H2K_stacks)
#if ARCHV <= 3
	r0 = ssr
	r0 = extractu(r0,#3,#19)
#else
	r0 = HTID
#endif
	r3 = #KERNEL_STACK_SIZE
	r29 += mpyi(r3,r0)
	r1:0 = #0
	k0lock
	jump H2K_switch
FUNC_END H2K_handle_reset

l2_sizes:
	.byte 0x0  //  0: 0K
	.byte 0x2  //  1: 128K
	.byte 0x3  //  2: 256k
	.byte 0x7  //  3: reserved
	.byte 0x4  //  4: 512K
	.byte 0x7  //  5: reserved
	.byte 0x4  //  6: 512K + 256K TCM
	.byte 0x7  //  7: reserved
	.byte 0x4  //  8: 512K + 512K TCM (or 0K + 1024K TCM ?)
	.byte 0x7  //  9: reserved
	.byte 0x7  //  10: reserved
	.byte 0x7  //  11: reserved
	.byte 0x7  //  12: reserved
	.byte 0x7  //  13: reserved
	.byte 0x7  //  14: reserved
	.byte 0x7  //  15: reserved

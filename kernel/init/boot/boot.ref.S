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

#define MAKE_TLBLO(DST,SRCVAL,TMPREG) \
	SETCONST(DST,BOOT_TLBLO) ; \
	TMPREG = lsr(SRCVAL, #  BOOT_TLB_OFFSET_BITS); \
	DST |= asl(TMPREG, #  BOOT_TLB_SHIFT+SBIT_SIZE);

#define MAKE_TLBLO_DEV(DST,SRCVAL,TMPREG) \
	SETCONST(DST,BOOT_TLBLO_DEV) ; \
	TMPREG = lsr(SRCVAL, #  BOOT_TLB_OFFSET_BITS_4M); \
	DST |= asl(TMPREG, #  BOOT_TLB_SHIFT_4M + SBIT_SIZE);

#define DATA       r1:0

#define INDEX        r1
#define TLB_ENTRY  r3:2  // TLB data
#define TLB_LO       r2
#define TLB_HI       r3
#define NPAGES       r4  // # kernel pages
#define PAGE_COUNT   r5  // counter for TLB setup
#define IMAGE_SIZE   r7
#define CFG_IDX      r8  // cfg_table index
#define CFG_HI       r9  // cfg_table address [35:11]
#define TMP         r10
#define TABLE_PTR   r11
#define TCM_BASE    r12  // TCM base from cfg_table
#define BOOT_OFFSET r13  // subtract from __bootvm_entry to get bootvm VA
#define SS_BASE     r14  // *public* subsystem base
#define REVID       r15  // rev
#define LAST_TLB    r16  // last replaceable TLB index
#define PHYS_ADDR   r17  // where we are loaded, page-aligned
#define L2_SIZE     r18  // L2 mem size from rev
#define REV_ARCH    r19  // arch from rev
#define REV_UARCH   r20  // uarch from rev
#define LINK_ADDR   r21  // kernel VA
#define COUNTER     r22
#define TMP_L2      r23
#define L2CFG       r24

#define SP          r29

FUNC_START start .entry .p2align 12
	jump 1f
	.space 0x40
1:
	 /* FIXME: Read syscfg and set bits, don't zero, in case earlier boot code
	has set it up */

	/* NOTE: 4096 bytes should contain all the way to jump to
	hyperspace... */
	TMP = #0					// 0
	syscfg = TMP				// 1
	ickill					// 2
	dckill					// 3
	l2kill					// 4
	nop            // 5
	nop 					 // 6
	TMP = #(SYSCFG_I | SYSCFG_D)					// 7
	syscfg = TMP				// 0
	isync					// 1 ... needs to be in same cache line as syscfg
	/* Setup EVB */
	SETCONST(TMP,H2K_event_vectors)
	evb = TMP

	SETCONST(TMP,_SDA_BASE_)	// boot thread should do this...
	gp = TMP

	SETCONST(H2K_GP,H2K_kg)

#if ARCHV == 3
	TMP.h = #0x0100
	TMP.l = #0x0000
	s60 = TMP
#elif ARCHV == 4
	TMP = ##0x00002004
	s63 = TMP
#elif ARCHV == 5
	TMP = #0     /* (New chickenbits) */
	TMP = setbit(TMP,#2);	//  AXI L1 Snoop something
	TMP = setbit(TMP,#27);	//  force store
	TMP = setbit(TMP,#12);	//  HW loops
	s63 = TMP
	TMP = #0
	TMP = setbit(TMP,#22);	//  fix for address calculation corruption
	s62 = TMP
#else
#endif

	/* Set up rising edge triggered */
	TMP = #-1
	/* Set imask to -1, since we are not lowest priority at boot */
 	/* (other idle threads) */
	imask = TMP
	iel = TMP
	iahl = TMP
	cswi(TMP)
	ciad(TMP)

	PHYS_ADDR = pc
	PHYS_ADDR = and(PHYS_ADDR, ##BOOT_TLB_PAGE_MASK)
	SETCONST(LINK_ADDR, H2K_LINK_ADDR)
	BOOT_OFFSET = sub(LINK_ADDR, PHYS_ADDR)
	SETCONST(NPAGES, H2K_KERNEL_NPAGES)
	REVID = rev
	REV_ARCH = and(REVID, #0xff)         // arch version
	REV_UARCH = extractu(REVID, #4, #8)  // uarch version
	L2_SIZE = extractu (REVID, #4, #12)  // L2 mem size

	// Read cfg_table
	CFG_HI = cfgbase
	CFG_HI = asl(CFG_HI, #(CFG_TABLE_SHIFT - PHYSREAD_HI_SHIFT))

	CFG_IDX = #CFG_TABLE_SSBASE
	SS_BASE = memw_phys(CFG_IDX, CFG_HI) // == SSBASE[35:16]
	SS_BASE = asl(SS_BASE, #CFG_TABLE_SHIFT)  // SSBASE

#ifdef H2K_USE_TCM

	// First see if there's room; only possible on v5
#if ARCHV <= 4
#error USE_TCM on <= v4?
#endif

	// Bail out if L2 size not 768K
	p0 = cmp.eq(L2_SIZE, #6)
	if (!p0) jump .Lthere

	CFG_IDX = #CFG_TABLE_L2TCM
	TCM_BASE = memw_phys(CFG_IDX, CFG_HI) // == L2TCM[35:16]
	TCM_BASE = asl(TCM_BASE, #CFG_TABLE_SHIFT)

	/* FIXME?  Go ahead and assume TCM is in lower 4G and aligned to at least
	kernel page size */

	// copy ourselves to TCM

#define ORG_PTR     r25  // TCM copy-from pointer
#define TCM_PTR     r27  // TCM copy-to pointer

	ORG_PTR = PHYS_ADDR  // first kernel page
	PHYS_ADDR = TCM_BASE
	SETCONST(IMAGE_SIZE, end)
	IMAGE_SIZE = sub(IMAGE_SIZE, LINK_ADDR)
	TCM_PTR = TCM_BASE
	COUNTER = #0	
3:
	DATA = memd(ORG_PTR)
	memd(TCM_PTR) = DATA
	ORG_PTR = add(ORG_PTR, #8)
	TCM_PTR = add(TCM_PTR, #8)
	COUNTER = add(COUNTER, #8)
	p0 = cmp.gt(COUNTER, IMAGE_SIZE)
	if (!p0) jump 3b

	// go there
	SETCONST(TCM_PTR, .Lthere)
	TCM_PTR = and(TCM_PTR, ##BOOT_TLB_OFFSET_MASK)  // offset of .Lthere
	TCM_PTR = or(TCM_BASE, TCM_PTR)
	jumpr TCM_PTR

.Lthere:

#undef ORG_PTR
#undef TCM_PTR

#endif

	p0 = cmp.eq(REV_ARCH, #0x60)  // v60?
	if (p0) jump .Lv60
	p0 = cmp.eq(REV_ARCH, #0x56)  // v60?
	if (p0) jump .Lv56

	// < v56
	p0 = cmp.eq(REV_UARCH, #2)  // v55c
	if (p0) jump .Ljtlb64
	p0 = cmp.eq(REV_UARCH, #3)  // v55d
	if (p0) jump .Ljtlb64

	LAST_TLB = #128            // all others
	jump .Ltlbsetup

.Lv56:
	p0 = cmp.eq(REV_UARCH, #6)  // v56t
	if (p0) jump .Ljtlb64
	p0 = cmp.eq(REV_UARCH, #8)  // v56u
	if (p0) jump .Ljtlb64

	LAST_TLB = #128            // all others
	jump .Ltlbsetup

.Lv60:
	p0 = cmp.eq(REV_UARCH, #2)  // v60c
	if (p0) jump .Ljtlb64
	p0 = cmp.eq(REV_UARCH, #5)  // v60f
	if (p0) jump .Ljtlb64

	LAST_TLB = #128            // all others
	jump .Ltlbsetup

.Ljtlb64:
	LAST_TLB = #64

.Ltlbsetup:
	/* Clear TLB */
	TLB_ENTRY = #0
	INDEX = #0
4:
	DO_TLBW(3,2,1)

	INDEX = add(INDEX,#1)
	p0 = cmp.eq(INDEX, LAST_TLB)  // # tlb entries
	if (!p0) jump 4b

	// Set up TLB entries for jump to virtual space
#define TLB_PHYS    r26  // phys page being mapped
#define TLB_TMP     r27

	TLB_PHYS = PHYS_ADDR

	INDEX = sub(LAST_TLB, NPAGES)
	INDEX = add(INDEX, #-1)  // starting index for pinned entries
	LAST_TLB = add(INDEX, #-1) // last free entry
	COUNTER = #0
	PAGE_COUNT = NPAGES

2:
	TMP = asl(COUNTER, #BOOT_TLB_OFFSET_BITS)
	SETCONST(TLB_HI, H2K_LINK_ADDR)
	TLB_HI = add(TLB_HI, TMP)
	TLB_HI = lsr(TLB_HI, #BOOT_TLB_OFFSET_BITS)
	TLB_HI = asl(TLB_HI, #BOOT_TLB_SHIFT)
	SETCONST(TMP, BOOT_TLBHI)
	TLB_HI = or(TLB_HI, TMP)
	MAKE_TLBLO(TLB_LO, TLB_PHYS, TLB_TMP)
	DO_TLBW(3,2,1)

	INDEX = add(INDEX, #1)
	COUNTER = add(COUNTER, #1)
	TLB_PHYS = PHYS_ADDR
	TLB_PHYS += asl(INDEX, #BOOT_TLB_OFFSET_BITS)
	
	PAGE_COUNT = add(PAGE_COUNT, #-1)
	p0 = cmp.eq(PAGE_COUNT, #0)
	if (!p0) jump 2b

#if ARCHV > 3
	// Set up entry for L2 interrupt controller
	MAKE_TLBHI(TLB_HI,Q6_SS_BASE_VA,BOOT_TLB_OFFSET_BITS_4M,BOOT_TLB_SHIFT_4M)
	// SS_BASE == SSBASE (PRIV_BASE)
	// apply offet to get PUB_BASE
	SS_BASE = add(SS_BASE, ##-QDSP6SS_PUB_PRIV_OFFSET)
	MAKE_TLBLO_DEV(TLB_LO, SS_BASE, TLB_TMP)
	DO_TLBW(3,2,1)

	// index for temporary entry
	INDEX = #0
#endif

	// Don't make a temporary entry if we're loaded at the link address,
	// to avoid multiple tlb match since the permanent entry will match.
	TMP = and(LINK_ADDR, PHYS_ADDR)
	p0 = cmp.eq(TMP, LINK_ADDR)
	if (p0) jump 1f

	/* This one page had better be enough */
	MAKE_TLBHI(TLB_HI,0,BOOT_TLB_OFFSET_BITS,BOOT_TLB_SHIFT)
	TLB_HI |= lsr(PHYS_ADDR, #BOOT_TLB_ADDRBITS)
	MAKE_TLBLO(TLB_LO, PHYS_ADDR, TLB_TMP)
	DO_TLBW(3,2,1)
1:
	/* Turn on translation, ISDB, PMU, ignore data aborts, TLB lock, etc */

#ifdef ISDBEN
	TMP = #1;
	isdben = TMP
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
 	TMP = #SYSCFG_LO_ALL
#else
	TMP = ##(SYSCFG_LO_ALL | SYSCFG_BQ | SYSCFG_DMT)
#endif

#define SYSCFG_HI_ALL (SYSCFG_L2WB)

#if ARCHV <= 3
	TMP.h = #((SYSCFG_HI_ALL | SYSCFG_L2CFG_256K) >> 16)
#elif ARCHV == 4
	TMP.h = #((SYSCFG_HI_ALL | SYSCFG_ITCM) >> 16)
#else // ARCHV >= 5
	TMP.h = #((SYSCFG_HI_ALL | SYSCFG_ITCM) >> 16)
#endif

	syscfg = TMP
	isync
	// make the jump to hyperspace
	TMP.h = #hi(1f)
	TMP.l = #lo(1f)
	jumpr TMP

1:
	// invalidate temporary TLB entry
	TLB_ENTRY = #0
	DO_TLBW(3,2,1)
	isync

	SETCONST(SP, H2K_stacks)

	r0 = sub(LINK_ADDR, PHYS_ADDR)  // phys_offset
	r1 = BOOT_OFFSET
	r2 = SS_BASE
	r3 = LAST_TLB

	jump H2K_thread_boot
FUNC_END start

FUNC_START h2_init .text.init.boot .p2align 2
	jumpr r31
FUNC_END h2_init

#undef INDEX
#define STACK_SIZE r3
FUNC_START H2K_handle_reset .text.init.boot .p2align 2
	SETCONST(TMP,_SDA_BASE_)
	gp = TMP

	SETCONST(H2K_GP,H2K_kg)

	SETCONST(SP ,H2K_stacks)
#if ARCHV <= 3
	TMP = ssr
	TMP = extractu(TMP,#3,#19)
#else
	TMP = HTID
#endif
	STACK_SIZE = #KERNEL_STACK_SIZE
	SP += mpyi(STACK_SIZE, TMP)
	DATA = #0
	k0lock
	jump H2K_switch
FUNC_END H2K_handle_reset

.p2align 2


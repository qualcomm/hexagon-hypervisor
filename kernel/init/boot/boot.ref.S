/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>
#include <boot.h>
#include <h2_common_pmap.h>

#define DO_TLBW \
	tlbw(TLB_ENTRY, TLB_INDEX)

#define SETCONST(DST,VAL) \
	DST = # ## # (VAL) ;

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

#define TLB_INDEX    r1
#define TLB_ENTRY  r3:2  // TLB data
#define TLB_LO       r2
#define TLB_HI       r3
#define NPAGES       r4  // # kernel pages
#define PAGE_COUNT   r5  // counter for TLB setup

#define CFG_IDX      r8  // cfg_table index
#define CFG_HI       r9  // cfg_table address [35:11]
#define TMP         r10
#define FROM        r11
#define TO          r12
#define TMP2        r13
#define SS_BASE     r14  // *public* subsystem base
#define SHIFT       r15  // multicore shift - offset translation for boot VM
#define LAST_TLB    r16  // last replaceable TLB index
#define PHYS_ADDR   r17  // where we are loaded, page-aligned
#define CORE_ID     r18
#define CORE_COUNT  r19  // 0 for non-multicore
#define BARRIER     r20
#define LINK_ADDR   r21  // kernel VA
#define COUNTER     r22
#define STACK_SIZE  r23
#define TLB_SIZE 		r24

#define TLB_PHYS    r26  // phys page being mapped
#define TLB_TMP     r27
// H2K_GP r28
#define SP          r29

FUNC_START start .entry .p2align 12
	jump 1f
	.space 0x80  // FIXME: needs to be enough for all angel mailboxes but still fit all code in 4K

1:

#ifdef CRASH_DEBUG
	r7 = PC
	jump 3f
H2_BOOT_attempts:
	.word 0
.Lreset_crash:
	r6 = add(PC,##H2K_kg@PCREL)
	r1 = HTID
	r4 = memw(r6+#KG_syscfg_val)
	r4 = and(r4,#0xFFFFFFEE) // no interrupts, no MMU
	syscfg = r4 // turn on caches just like they were
	isync
	r0 = add(r6,#KG_crash_contexts)
	r0 += mpyi(r1,##CONTEXT_SIZE)
	r2 = cs0
	memd(r0+#CONTEXT_r3130) = r31:30
	memd(r0+#CONTEXT_r2928) = r29:28
	memd(r0+#CONTEXT_r2726) = r27:26
	memd(r0+#CONTEXT_r2524) = r25:24
	memd(r0+#CONTEXT_r2322) = r23:22
	memd(r0+#CONTEXT_r2120) = r21:20
	memd(r0+#CONTEXT_r1918) = r19:18
	memd(r0+#CONTEXT_r1716) = r17:16
	memw(r0+#CONTEXT_elr) = r2
	H2K_GP = r6
	r5:4 = s1:0
	memd(r0+#CONTEXT_cs1cs0) = r5:4
	r29 = add(PC,##H2K_stacks@PCREL)
	r29 += mpyi(r1,#KERNEL_STACK_SIZE)
	r0 = add(H2K_GP,#KG_crash_tlb)
	loop0(9f,#MAX_TLB_ENTRIES)
	r1 = #0
9:
	r3:2 = tlbr(r1)
	{
		memd(r0++#8) = r3:2
		r1 = add(R1,#1)
	}:endloop0
	nop
	/* EJP: insert here: start other threads, have them save off their own stuff, then stop */
	call H2K_cache_d_cleaninv
	call H2K_cache_l2_cleaninv
	syncht
	nop
8:
	stop(r0)
	jump 8b
3:
	r0 = memw(r7+#8)
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) jump:nt .Lreset_crash
		r0 = add(r0,#1)
		memw(r7+#8) = r0.new
	}
#endif

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

	PHYS_ADDR = pc
	PHYS_ADDR = and(PHYS_ADDR, ##BOOT_TLB_PAGE_MASK)  // addr of lowest page
	SETCONST(LINK_ADDR, H2K_LINK_ADDR)
	SETCONST(NPAGES, H2K_KERNEL_NPAGES)

	// Read cfg_table
	CFG_HI = cfgbase
	CFG_HI = asl(CFG_HI, #(CFG_TABLE_SHIFT - PHYSREAD_HI_SHIFT))

	CFG_IDX = #CFG_TABLE_SSBASE
	SS_BASE = memw_phys(CFG_IDX, CFG_HI) // == SSBASE[35:16]
	SS_BASE = asl(SS_BASE, #CFG_TABLE_SHIFT)  // SSBASE

#if ARCHV >= 68
	CFG_IDX = #CFG_TABLE_JTLB_SIZE
	TLB_SIZE = memw_phys(CFG_IDX, CFG_HI)
	LAST_TLB = TLB_SIZE
#endif

#ifdef MULTICORE
	CFG_IDX = #CFG_TABLE_CORE_ID
	CORE_ID = memw_phys(CFG_IDX, CFG_HI)

	/*
	p0 = cmp.eq(CORE_ID, #0)
	if (!p0) jump 12f
	wait(r1)
12:
*/

	CFG_IDX = #CFG_TABLE_CORE_COUNT
	CORE_COUNT = memw_phys(CFG_IDX, CFG_HI)
	BARRIER = add(pc, ##boot_sync@PCREL)

	p0 = cmp.eq(CORE_COUNT, #0) // no multicore
	if (p0) jump 6f
	p0 = cmp.eq(CORE_ID, #0)
	if (!p0) jump 10f           // not core 0

	/* core 0 inits barrier */
	TMP = ct0(CORE_COUNT)       // FIXME: handle non-2^^n
	TMP = sub(#31, TMP)
	TMP2 = ##0xffffffff
	TMP2 = lsr(TMP2, TMP)
	memw(BARRIER) = TMP2        // other cores can go when ready
	dccleana(BARRIER)
11:	
	TMP = memw(BARRIER)
	p0 = cmp.eq(TMP, #0x1)      // other cores done
	if (!p0) jump 11b
	jump 6f

	/* Clone for multicore, cores 1 - N */
10:
	PAGE_COUNT = add(NPAGES, #1)        // FIXME: handle bootvm > 1 pages or different size
	FROM = PHYS_ADDR
	TO = lsr(PHYS_ADDR, #PAGE_BITS)     // 4K page
	TO = sub(#0x00100000, TO)           // total space
	TMP = ct0(CORE_COUNT)               // FIXME: handle non-2^^n
	TO = lsr(TO, TMP)
	TO = asl(TO, #PAGE_BITS)            // space per core
	TO = and(TO, ##BOOT_TLB_PAGE_MASK)  // align down to page size
	SHIFT = mpyi(TO, CORE_ID)           // total shift for this core
	TO = add(FROM, SHIFT)               // copy dest

	TMP2 = #(H2K_PAGESIZE >> 3)
	TMP = mpyi(PAGE_COUNT, TMP2)    // dwords to copy
	TMP2 = TO                       // save for jump

	loop0(7f, TMP)
7:
	{
		DATA = memd(FROM)
		FROM = add(FROM, #8)
	}
	{
		memd(TO) = DATA
		TO = add(TO, #8)
	}:endloop0
13:
	TMP = memw(BARRIER)
	p0 = cmp.eq(TMP, ##0xffffffff)  // wait for core 0 barrier init
	if (p0) jump 13b
12:
	TMP = memw_locked(BARRIER)
	TMP = clrbit(TMP, CORE_ID)      // copy done for this core
	memw_locked(BARRIER, p0) = TMP
	if (!p0) jump 12b
	dccleana(BARRIER)

	TMP = ##boot_continuation
	TMP = and (TMP, #BOOT_TLB_OFFSET_MASK)
	TMP = add(TMP, TMP2)
	jumpr TMP                         // point pc to copied page
6:
boot_continuation:
	PHYS_ADDR = pc
	PHYS_ADDR = and(PHYS_ADDR, ##BOOT_TLB_PAGE_MASK)  // addr of lowest page
#endif
	
	/* Setup EVB */
	SETCONST(TMP,H2K_event_vectors)
	evb = TMP

	SETCONST(TMP,_SDA_BASE_)	// boot thread should do this...
	gp = TMP

	SETCONST(H2K_GP,H2K_kg)

	/* Set up rising edge triggered */
	TMP = #-1
	/* Set imask to -1, since we are not lowest priority at boot */
 	/* (other idle threads) */
	imask = TMP
#if ARCHV < 65
	iel = TMP
	iahl = TMP
#endif
	cswi(TMP)
	ciad(TMP)

	// Clear TLB and probe size.  We rely on the index wrapping around in ARCHV < 68!
	TLB_ENTRY = #0
#if ARCHV < 68
	TLB_INDEX = #1
#else
	TLB_INDEX = #0
#endif

4:
	DO_TLBW

	TLB_INDEX = add(TLB_INDEX,#1)
#if ARCHV < 68
	p0 = cmp.eq(TLB_INDEX, #MAX_TLB_ENTRIES)  // # tlb entries
#else
	p0 = cmp.eq(TLB_INDEX, TLB_SIZE)  // # tlb entries
#endif
	if (!p0) jump 4b

#if ARCHV < 68
	// mark entry 0 so we can tell when index wraps
	TLB_ENTRY = #1
	TLB_INDEX = #0
	DO_TLBW

5:
	TLB_INDEX = add(TLB_INDEX,#1)
	TLB_ENTRY = tlbr(TLB_INDEX)
	p0 = cmp.eq(TLB_LO, #1)
	if (!p0) jump 5b
	LAST_TLB = TLB_INDEX
	TLB_SIZE = TLB_INDEX
#endif

	// Set up TLB entries for jump to virtual space
	TLB_PHYS = PHYS_ADDR

	TLB_INDEX = sub(LAST_TLB, NPAGES)
	TLB_INDEX = add(TLB_INDEX, #-1)  // -1 for device page = starting index for pinned entries
	LAST_TLB = add(TLB_INDEX, #-1) // last free entry
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
	DO_TLBW

	TLB_INDEX = add(TLB_INDEX, #1)
	COUNTER = add(COUNTER, #1)
	TLB_PHYS = PHYS_ADDR
	TLB_PHYS += asl(COUNTER, #BOOT_TLB_OFFSET_BITS)
	
	PAGE_COUNT = add(PAGE_COUNT, #-1)
	p0 = cmp.eq(PAGE_COUNT, #0)
	if (!p0) jump 2b

	// Set up entry for L2 interrupt controller
	MAKE_TLBHI(TLB_HI,Q6_SS_BASE_VA,BOOT_TLB_OFFSET_BITS_4M,BOOT_TLB_SHIFT_4M)
	// SS_BASE == SSBASE (PRIV_BASE)
	// apply offet to get PUB_BASE
	SS_BASE = add(SS_BASE, ##-QDSP6SS_PUB_PRIV_OFFSET)
	MAKE_TLBLO_DEV(TLB_LO, SS_BASE, TLB_TMP)
	DO_TLBW

	// index for temporary entry
	TLB_INDEX = #0

	// Don't make a temporary entry if we're loaded at the link address,
	// to avoid multiple tlb match since the permanent entry will match.
	TMP = and(LINK_ADDR, PHYS_ADDR)
	p0 = cmp.eq(TMP, LINK_ADDR)
	if (p0) jump 1f

	/* This one page had better be enough */
	MAKE_TLBHI(TLB_HI,0,BOOT_TLB_OFFSET_BITS,BOOT_TLB_SHIFT)
	TLB_HI |= lsr(PHYS_ADDR, #BOOT_TLB_ADDRBITS)
	MAKE_TLBLO(TLB_LO, PHYS_ADDR, TLB_TMP)
	DO_TLBW
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

#if ARCHV == 4
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
	DO_TLBW
	isync

	r0 = syscfg
	r1 = add(PC,##H2K_kg@PCREL)
	memw(r1+#KG_syscfg_val) = r0

	SETCONST(SP, H2K_stacks)

	// make sure these are initialized
	FP = #0
	LR = #0

	r0 = SHIFT
	r1 = SS_BASE
	r2 = LAST_TLB
	r3 = TLB_SIZE
	r4 = CORE_ID
	r5 = CORE_COUNT

	jump H2K_thread_boot
FUNC_END start

	.p2align 8
boot_sync:
	.word 0xffffffff  // ensure non-0

FUNC_START h2_init .text.init.boot .p2align 2
	jumpr r31
FUNC_END h2_init

FUNC_START H2K_handle_reset .text.init.boot .p2align 2
	SETCONST(TMP,_SDA_BASE_)
	gp = TMP

	SETCONST(H2K_GP,H2K_kg)

	SETCONST(SP ,H2K_stacks)
	TMP = HTID
	STACK_SIZE = #KERNEL_STACK_SIZE
	SP += mpyi(STACK_SIZE, TMP)
	r1:0 = #0
	k0lock
	jump H2K_switch
FUNC_END H2K_handle_reset

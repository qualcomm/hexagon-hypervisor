/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

#if __QDDSP6_ARCH__ <= 3
#define DO_TLBW(HI,LO,IDX) \
	tlbidx = R##IDX ; \
	tlblo = R##LO ; \
	tlbhi = R##HI ; \
	tlbw ;
#else
#define DO_TLBW(HI,LO,IDX) \
	tlbw(R##HI##:##LO,R##IDX)
#endif

#if __QDSP6_ARCH__ <= 3
#define SETCONST(DST,VAL) \
	DST.H = #hi(VAL); \
	DST.L = #lo(VAL); 
#else
#define SETCONST(DST,VAL) \
	DST = ##VAL ;
#endif

#define MAKE_TLBHI(DST,CONSTVA) \
	SETCONST(DST,BOOT_TLBHI | (CONSTVA >> ADDRBITS))

FUNC_START start .text.init.boot .p2align 8
	r0 = #0					// 0
	syscfg = r0				// 1
	ickill					// 2
	dckill					// 3
	l2kill					// 4
	r15.h = #hi(H2K_bootmap_ptr)		// 5
	r15.l = #lo(H2K_bootmap_ptr)		// 6
	r0 = #6					// 7
	syscfg = r0				// 0
	isync					// 1 ... needs to be in same cache line as syscfg
	/* Setup EVB */
	SETCONST(r0,H2K_event_vectors)
	evb = r0

	SETCONST(r0,_SDA_BASE_)	// boot thread should do this...
	gp = r0

	SETCONST(r16,H2K_kg)

#if __QDSP6_ARCH__ == 3
	r0.h = #0x0100
	r0.l = #0x0000
	s60 = r0
#elif __QDSP6_ARCH__ == 4
	r0 = ##0x00002004
	s63 = r0
#elif __QDSP6_ARCH__ == 5
	r0 = #0     /* (No chickenbits) */
	s63 = r0
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

	/* Clear TLB */
	r3:2 = #0
	r1 = #0
1:
	DO_TLBW(3,2,1)

	r1 = add(r1,#1)
	p0 = cmp.eq(r1,#MAX_TLB_ENTRIES)
	if (!p0) jump 1b

	// Set up TLB entries for jump to virtual space
	r10 = pc

	MAKE_TLBHI(r3,H2K_LINK_ADDR)
	MAKE_TLBLO(r2,r11)

	r2 = lsr(r10,#BOOT_TLB_PGBITS)
#if __QDSP6_ARCH__ <= 3
	r2 = asl(r2,#BOOT_TLB_SHIFT)
#else
	r2 = asl(r2,#(BOOT_TLB_SHIFT + 1))  // room for S bit
#endif
	r3.h = #hi(BOOT_TLBHI | (BOOT_TLB_PGNUM + (0 << BOOT_TLB_SHIFT)))
	r3.l = #lo(BOOT_TLBHI | (BOOT_TLB_PGNUM + (0 << BOOT_TLB_SHIFT)))
	r11.h = #hi(BOOT_TLBLO)
	r11.l = #lo(BOOT_TLBLO)
	r2 = or(r2,r11)

	r1 = #0
	DO_TLBW(3,2,1)

	r1 = #1
	r2 = lsr(r10,#BOOT_TLB_PGBITS)
	r2 = add(r2,#1)
#if __QDSP6_ARCH__ <= 3
	r2 = asl(r2,#BOOT_TLB_SHIFT)
#else
	r2 = asl(r2,#(BOOT_TLB_SHIFT + 1))  // room for S bit
#endif
	r3.h = #hi(BOOT_TLBHI | (BOOT_TLB_PGNUM + (1 << BOOT_TLB_SHIFT)))
	r3.l = #lo(BOOT_TLBHI | (BOOT_TLB_PGNUM + (1 << BOOT_TLB_SHIFT)))

	r11.h = #hi(BOOT_TLBLO)
	r11.l = #lo(BOOT_TLBLO)

	r2 = or(r2,r11)
	DO_TLBW(3,2,1)

#if __QDSP6_ARCH__ > 3
	// Set up entry for L2 interrupt controller
	r1 = #2
	r2.h = #hi((L2_INT_BASE >> 24) << 12)
	r2.l = #lo((L2_INT_BASE >> 24) << 12)
	r3.h = #hi(BOOT_TLBHI)
	r3.l = #lo(BOOT_TLBHI)
	r3 = or(r3, r2)
	r11.h = #hi((1 << 6) | (4 << TLB_ENTRY_C_BITS))  // 16M page
	r11.l = #lo((1 << 6) | (4 << TLB_ENTRY_C_BITS))

	r2 = or(r2,r11)
	tlbw(r3:2,r1)
#endif

  // Set up temporary entries for current space
	r1 = #3
	r2 = lsr(r10,#BOOT_TLB_PGBITS)
	r2 = asl(r2,#BOOT_TLB_SHIFT)

	// skip this if virtual == physical for monitor space			
	r12.h = #hi(BOOT_TLB_PGNUM)
	r12.l = #lo(BOOT_TLB_PGNUM)
	p0 = cmp.eq(r2, r12)
	if (p0) jump 1f

	r3.h = #hi(BOOT_TLBHI)
	r3.l = #lo(BOOT_TLBHI)
	r3 = or(r3, r2)
#if __QDSP6_ARCH__ > 3
	r2 = asl(r2,#1)  // room for S bit
#endif
	r11.h = #hi(BOOT_TLBLO)
	r11.l = #lo(BOOT_TLBLO)

	r2 = or(r2,r11)
	DO_TLBW(3,2,1)

	r1 = #4
	r2 = lsr(r10,#BOOT_TLB_PGBITS)
	r2 = add(r1,#1)
	r2 = asl(r2,#BOOT_TLB_SHIFT)
	r3.h = #hi(BOOT_TLBHI)
	r3.l = #lo(BOOT_TLBHI)
	r3 = or(r3, r2)
#if __QDSP6_ARCH__ > 3
	r2 = asl(r2,#1)  // room for S bit
#endif
	r11.h = #hi(BOOT_TLBLO)
	r11.l = #lo(BOOT_TLBLO)

	r2 = or(r2,r11)
	DO_TLBW(3,2,1)

1:
	/* Turn on translation, ISDB, PMU, ignore data aborts, TLB lock, etc */
#if __QDSP6_ARCH__ <= 4
 	r0 = #0x77f
#else
	r0 = ##0xA77F    /* DMT and non-QoS */
 	//r0 = ##0x877F    /* DMT and QoS */
	//r0 = #0x277F    /* IMT and non-QoS */
 	//r0 = #0x77F    /* IMT and QoS */
#endif

#if __QDSP6_ARCH__ <= 3
	r0.h = #0x0083	/* 256KB L2 */
#else
	r0.h = #0x008b	/* 256KB L2 + ITCM */
#endif
	syscfg = r0
	isync

	// make the jump to hyperspace
	r1.h = #hi(1f)
	r1.l = #lo(1f)
	jumpr r1

1:
	// invalidate temporary TLB entries
	r1 = #3
	r3:2 = #0
	DO_TLBW(3,2,1)

	r1 = #4
	r3:2 = #0
	DO_TLBW(3,2,1)
	isync

	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	// lock kernel

	k0lock

	// Wake up other threads
	r0 = #(1<<MAX_HTHREADS)-1
	start(r0)

	jump H2K_thread_boot
FUNC_END start

FUNC_START h2_init .text.init.boot .p2align 2
	jumpr r31
FUNC_END h2_init

FUNC_START H2K_handle_reset .text.init.boot .p2align 2
	r0.h = #hi(_SDA_BASE_)
	r0.l = #lo(_SDA_BASE_)
	gp = r0

	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)

	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
#if __QDSP6_ARCH__ <= 3
	r28 = ssr
	r28 = extractu(r28,#3,#19)
#else
	r28 = HTID
#endif
	r3 = #KERNEL_STACK_SIZE
	r29 += mpyi(r3,r28)
	r1:0 = #0
	k0lock
	jump H2K_switch
FUNC_END H2K_handle_reset


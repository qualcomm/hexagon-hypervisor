/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>
#include <boot.h>
#include <bootmap_macros.h>

#if __QDSP6_ARCH__ <= 3
#define DO_TLBW(HI,LO,IDX) \
	tlbidx = r##IDX ; \
	tlblo = r##LO ; \
	tlbhi = r##HI ; \
	tlbw ;
#else
#define DO_TLBW(HI,LO,IDX) \
	tlbw(R##HI##:##LO,R##IDX)
#endif

#if __QDSP6_ARCH__ <= 3
#define SETCONST(DST,VAL) \
	DST.H = # hi(VAL); \
	DST.L = # lo(VAL); 
#else
#define SETCONST(DST,VAL) \
	DST = # ## # (VAL) ;
#endif

#define MAKE_TLBHI(DST,CONSTVA) \
	SETCONST(DST,BOOT_TLBHI | (((CONSTVA) >> BOOT_TLB_PGBITS)<<BOOT_TLB_SHIFT))

#define MAKE_TLBHI_4K(DST,CONSTVA) \
	SETCONST(DST,BOOT_TLBHI | (((CONSTVA) >> BOOT_TLB_PGBITS_4K)<<BOOT_TLB_SHIFT_4K))

#define MAKE_TLBLO(DST,SRCVAL,TMP) \
	SETCONST(DST,BOOT_TLBLO) ; \
	TMP = lsr(SRCVAL, #  BOOT_TLB_PGBITS); \
	DST |= asl(TMP, #  BOOT_TLB_SHIFT+SBIT_SIZE); 

#define MAKE_TLBLO_DEV(DST,SRCVAL,TMP) \
	SETCONST(DST,BOOT_TLBLO_DEV) ; \
	TMP = lsr(SRCVAL, #  BOOT_TLB_PGBITS); \
	DST |= asl(TMP, #  BOOT_TLB_SHIFT+SBIT_SIZE); 

#define MAKE_TLBLO_DEV_4K(DST,SRCVAL,TMP) \
	SETCONST(DST,BOOT_TLBLO_DEV_4K) ; \
	TMP = lsr(SRCVAL, #  BOOT_TLB_PGBITS_4K); \
	DST |= asl(TMP, #  BOOT_TLB_SHIFT_4K+SBIT_SIZE); 

FUNC_START start .text.init.boot .p2align 12
	/* NOTE: 4096 bytes should contain all the way to jump to hyperspace... */
	r0 = #0					// 0
	syscfg = r0				// 1
	ickill					// 2
	dckill					// 3
	l2kill					// 4
	//	r15.h = #hi(H2K_bootmap_ptr)		// 5
	//	r15.l = #lo(H2K_bootmap_ptr)		// 6
	nop            // 5
	nop 					 // 6
	r0 = #6					// 7
	syscfg = r0				// 0
	isync					// 1 ... needs to be in same cache line as syscfg
	/* Setup EVB */
	SETCONST(r0,H2K_event_vectors)
	evb = r0

	SETCONST(r0,_SDA_BASE_)	// boot thread should do this...
	gp = r0

	SETCONST(H2K_GP,H2K_kg)

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
	MAKE_TLBLO(r2,r10,r11)
	r1 = #0
	DO_TLBW(3,2,1)

	r1 = #1
	r10 += asl(r1,#BOOT_TLB_PGBITS)
	MAKE_TLBHI(r3,H2K_LINK_ADDR+(1<<BOOT_TLB_PGBITS))
	MAKE_TLBLO(r2,r10,r11)
	DO_TLBW(3,2,1)

#if __QDSP6_ARCH__ > 3
	// Set up entry for L2 interrupt controller
#if 0
	SETCONST(r10,Q6_SS_BASE_PA)
	MAKE_TLBHI(r3,Q6_SS_BASE_VA)
	MAKE_TLBLO_DEV(r2,r10,r11)
	r1 = #2
	DO_TLBW(3,2,1)
#endif

	SETCONST(r10,TIMER_BASE_PA)
	MAKE_TLBHI_4K(r3,TIMER_BASE_VA)
	MAKE_TLBLO_DEV_4K(r2,r10,r11)
	r1 = #2
	DO_TLBW(3,2,1)
  
	SETCONST(r10,(TIMER_BASE_PA + 0x1000))
	MAKE_TLBHI_4K(r3,(TIMER_BASE_VA + 0x1000))
	MAKE_TLBLO_DEV_4K(r2,r10,r11)
	r1 = #3
	DO_TLBW(3,2,1)
  
	SETCONST(r10,L2_INT_BASE_PA)
	MAKE_TLBHI_4K(r3,L2_INT_BASE)
	MAKE_TLBLO_DEV_4K(r2,r10,r11)
	r1 = #4
	DO_TLBW(3,2,1)
  
#endif

	r10 = pc
	SETCONST(r11,H2K_LINK_ADDR)
	r12 = and(r11,r10)
	p0 = cmp.eq(r12,r11)
	if (p0) jump 1f

	MAKE_TLBHI(r3,0)
	r3 |= lsr(r10,#BOOT_TLB_ADDRBITS)
	MAKE_TLBLO(r2,r10,r11)
	r1 = #5
	DO_TLBW(3,2,1)
1:
	/* Turn on translation, ISDB, PMU, ignore data aborts, TLB lock, etc */
	r0 = #1;
	isdben = r0

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
	r1 = #5
	r3:2 = #0
	DO_TLBW(3,2,1)
	isync

	SETCONST(r29,H2K_stacks)
	// lock kernel

	k0lock

	// Wake up other threads
	r0 = #(1<<MAX_HTHREADS)-1
	start(r0)

	// r10 still has physical pc.  Calculate offset for boot vm pmap
	r10 = lsr(r10, #BOOT_TLB_PGBITS)
	r10 = asl(r10, #BOOT_TLB_PGBITS)
	SETCONST(r11, H2K_LINK_ADDR)
	r0 = sub(r11, r10)

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
#if __QDSP6_ARCH__ <= 3
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


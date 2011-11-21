/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

FUNC_START start .text.init.boot .p2align 5
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
	r0.h = #hi(H2K_event_vectors)
	r0.l = #lo(H2K_event_vectors)
	evb = r0

	r0.h = #hi(_SDA_BASE_)
	r0.l = #lo(_SDA_BASE_)
	gp = r0

	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)

#if __QDSP6_ARCH__ == 3
	r0.h = #0x0100
	r0.l = #0x0000
	s60 = r0
#elif __QDSP6_ARCH__ == 4
	r0 = ##0x00002004
	s63 = r0
#else
#endif

	/* Set up rising edge triggered */
	r0 = #-1
	iel = r0
	iahl = r0
	cswi(r0)
	ciad(r0)

	/* Clear TLB */
	r3:2 = #0
	r1 = #0
1:
#if __QDSP6_ARCH__ <= 3
	tlbidx = r1
	tlblo = r2
	tlbhi = r3
	tlbw
#else
	tlbw(r3:2,r1)
#endif
	r1 = add(r1,#1)
	p0 = cmp.eq(r1,#MAX_TLB_ENTRIES)
	if (!p0) jump 1b

	r1 = #0
	r15 = memw(r15)
	/* Initialize TLB */
1:
	r14 = addasl(r15,r1,#3)
	r3:2 = memd(r14)
	p0 = cmp.eq(r3,#0)
	if (p0) jump 2f
#if __QDSP6_ARCH__ <= 3
	tlbidx = r1
	tlblo = r2
	tlbhi = r3
	tlbw
#else
	tlbw(r3:2,r1)
#endif
	r1 = add(r1,#1)
	p0 = cmp.eq(r1,#MAX_TLB_ENTRIES)
	if (!p0) jump 1b
	jump 2f
	.p2align 5
2:
	/* Turn on translation, ISDB, PMU, ignore data aborts, TLB lock, etc */
	r0 = #0x77f
#if __QDSP6_ARCH__ <= 3
	r0.h = #0x0083	/* 256KB L2 */
#else
	r0.h = #0x008b	/* 256KB L2 + ITCM */
#endif
	syscfg = r0
	isync

#if __QDSP6_ARCH__ == 4
	r0.h = #0xffff
	r0.l = #0
	ccr = r0
#endif

	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	// lock kernel

	k0lock

	// Wake up other threads
	r0 = #-1
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

#if __QDSP6_ARCH__ == 4
	r0.h = #0xffff
	r0.l = #0
	ccr = r0
#endif

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


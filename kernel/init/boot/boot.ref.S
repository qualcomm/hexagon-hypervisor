/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

FUNC_START start .text.init.boot .p2align 2
	r0 = #0
	syscfg = r0
	ickill
	dckill
	l2kill
	r15.h = #hi(H2K_bootmap_ptr)
	r15.l = #lo(H2K_bootmap_ptr)
	r0 = #6
	syscfg = r0
	/* Setup EVB */
	r0.h = #hi(H2K_event_vectors)
	r0.l = #lo(H2K_event_vectors)
	evb = r0

	r0.h = #hi(_SDA_BASE_)
	r0.l = #lo(_SDA_BASE_)
	gp = r0

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
2:
	/* Turn on translation, ISDB, etc */
	r0 = #0x7f
	syscfg = r0

	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	// lock kernel

	k0lock

	// Wake up other threads
	r0 = #-1
	start(r0)

	jump H2K_thread_boot

FUNC_START h2_init .text.init.boot .p2align 2
	jumpr r31
FUNC_END h2_init

FUNC_START H2K_handle_reset .text.init.boot .p2align 2
	r0.h = #hi(_SDA_BASE_)
	r0.l = #lo(_SDA_BASE_)
	gp = r0
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
	r1:0 = xor(r1:0,r1:0)
	k0lock
	jump H2K_switch
FUNC_END H2K_handle_reset

DATA_START H2K___refs .data.init.boot .p2align 2
	.word H2K_futex_wait
	.word H2K_futex_resume
	.word H2K_event_vectors
	/* TODO: fill in all refs */
DATA_END H2K___refs


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
	r15.h = #hi(BLASTK_bootmap_ptr)
	r15.l = #lo(BLASTK_bootmap_ptr)
	r0 = #6
	syscfg = r0

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

	r29.h = #hi(BLASTK_stacks)
	r29.l = #lo(BLASTK_stacks)

	jump BLASTK_thread_boot

FUNC_START blast_init .text.init.boot .p2align 2
	jumpr r31
FUNC_END blast_init

FUNC_START BLASTK_handle_reset .text.init.boot .p2align 2
	r29.h = #hi(BLASTK_stacks)
	r29.l = #lo(BLASTK_stacks)
#if __QDSP6_ARCH__ <= 3
	r28 = ssr
	r28 = extractu(r28,#3,#19)
#else
	r28 = TNUM
#endif
	r3 = #KERNEL_STACK_SIZE
	r29 += mpyi(r3,r28)
	r1:0 = xor(r1:0,r1:0)
	jump BLASTK_switch
FUNC_END BLASTK_handle_reset

DATA_START BLASTK___refs .data.init.boot .p2align 2
	.word BLASTK_futex_wait
	.word BLASTK_futex_resume
	.word BLASTK_event_vectors
	/* TODO: fill in all refs */
DATA_END BLASTK___refs


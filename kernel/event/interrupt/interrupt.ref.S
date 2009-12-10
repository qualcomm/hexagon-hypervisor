/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_handle_int .text.event.interrupt .p2align 2
	/* If SGP is null, we've interrupted waitmode. */
	crswap(r0,sgp)
	if (r0==#0) jump:t H2K_interrupted_waitmode

	/* Save off all context */
	/* Optimization Note:
	 * First, we should (maybe) dcfetch r0 and r0+32 
	 * Next, we should add 64 to r0 and dczeroa
	 * Then continue to dczeroa the rest of the cache lines
	 * Finally, we should reset r0 back to the original point 
	 * so the context offset macros work. 
	 */
	memd(r0+#CONTEXT_r3130) = r31:30
	memd(r0+#CONTEXT_r2928) = r29:28
	memd(r0+#CONTEXT_r2726) = r27:26
	memd(r0+#CONTEXT_r2524) = r25:24
	memd(r0+#CONTEXT_r2322) = r23:22
	memd(r0+#CONTEXT_r2120) = r21:20
	memd(r0+#CONTEXT_r1918) = r19:18
	memd(r0+#CONTEXT_r1716) = r17:16
	memd(r0+#CONTEXT_r1514) = r15:14
	memd(r0+#CONTEXT_r1312) = r13:12
	memd(r0+#CONTEXT_r1110) = r11:10
	memd(r0+#CONTEXT_r0908) = r9:8
	memd(r0+#CONTEXT_r0706) = r7:6
	memd(r0+#CONTEXT_r0504) = r5:4
	memd(r0+#CONTEXT_r0302) = r3:2
	r13 = ugp
	r12 = gp
	r11 = usr
	r10 = p3:0
	r9 = lc0
	r8 = sa0
	r7 = lc1
	r6 = sa1
	r5 = m1
	r4 = m0
	r3 = ssr
	r2 = elr
	memd(r0+#CONTEXT_ugpgp) = r13:12
	memd(r0+#CONTEXT_sr_preds) = r11:10
	memd(r0+#CONTEXT_lc0sa0) = r9:8
	memd(r0+#CONTEXT_lc1sa1) = r7:6
	memd(r0+#CONTEXT_m1m0) = r5:4
	memd(r0+#CONTEXT_ssrelr) = r3:2

	/* Save off last registers */
	r21 = r0
	crswap(r0,sgp)
	memd(r21+#CONTEXT_r0100) = r1:0

	/* All registers saved off, set up stack */

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r2 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r2)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif

	/* Stack set up, execute interrupt handler */
	r2 = r28 	// TNUM from above...
	r0 = ssr
	r0 = zxtb(r0)
	r1 = r21	// thread context
	r4.h = #hi(H2K_inthandlers)
	r4.l = #lo(H2K_inthandlers)
	r4 = addasl(r4,r0,#2)
	r4 = memw(r4)

	/* Registers set up, call specified handler */

	call .Lint_jump_stub // will jump to r4

H2K_interrupt_restore:
	/* Restore all state and return */
	r0 = sgp
	r3:2 = memd(r0+#CONTEXT_ssrelr)
	r5:4 = memd(r0+#CONTEXT_m1m0)
	r7:6 = memd(r0+#CONTEXT_lc1sa1)
	r9:8 = memd(r0+#CONTEXT_lc0sa0)
	r11:10 = memd(r0+#CONTEXT_sr_preds)
	r13:12 = memd(r0+#CONTEXT_ugpgp)
	elr = r2
	ssr = r3
	m0 = r4
	m1 = r5
	sa1 = r6
	lc1 = r7
	sa0 = r8
	lc0 = r9
	p3:0 = r10
	usr = r11
	gp = r12
	ugp = r13
	r3:2 = memd(r0+#CONTEXT_r0302)
	r5:4 = memd(r0+#CONTEXT_r0504)
	r7:6 = memd(r0+#CONTEXT_r0706)
	r9:8 = memd(r0+#CONTEXT_r0908)
	r11:10 = memd(r0+#CONTEXT_r1110)
	r13:12 = memd(r0+#CONTEXT_r1312)
	r15:14 = memd(r0+#CONTEXT_r1514)
	r17:16 = memd(r0+#CONTEXT_r1716)
	r19:18 = memd(r0+#CONTEXT_r1918)
	r21:20 = memd(r0+#CONTEXT_r2120)
	r23:22 = memd(r0+#CONTEXT_r2322)
	r25:24 = memd(r0+#CONTEXT_r2524)
	r27:26 = memd(r0+#CONTEXT_r2726)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r31:30 = memd(r0+#CONTEXT_r3130)
	r1:0 = memd(r0+#CONTEXT_r0100)
	rte
	.global H2K_interrupt_restore
FUNC_END H2K_handle_int

FUNC_START H2K_interrupted_waitmode .text.event.interrupt .p2align 2
#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r2 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r2)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif
	

	/* Stack set up, execute interrupt handler */
	r2 = r28 	// TNUM from above...
	r0 = ssr
	r0 = zxtb(r0)
	r1 = #0		// thread context == NULL
	r4.h = #hi(H2K_inthandlers)
	r4.l = #lo(H2K_inthandlers)
	r4 = addasl(r4,r0,#2)
	r4 = memw(r4)

	/* Registers set up, call specified handler */
	callr r4
	r0 = #0
	r1 = #0
	jump H2K_switch

.Lint_jump_stub:
	memw(r1+#CONTEXT_continuation) = r31
	jumpr r4

FUNC_END H2K_interrupted_waitmode


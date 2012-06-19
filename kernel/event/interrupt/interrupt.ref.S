/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_handle_int .text.core.interrupt .p2align 2
	/* If SGP is null, we've interrupted waitmode. */
	crswap(r0,sgp)
	if (r0==#0) jump:t H2K_interrupted_waitmode

	/* If SGP is 0x00000001, we've interrupted the preemption point */
	r0 = and(r0,#-2)
	if (r0 == #0) jump:nt H2K_interrupted_preempt

	/* Save off all context */
	/* Optimization Note:
	 * First, we should (maybe) dcfetch r0 and r0+32 
	 * Next, we should add 64 to r0 and dczeroa
	 * Then continue to dczeroa the rest of the cache lines
	 * Finally, we should reset r0 back to the original point 
	 * so the context offset macros work. 
	 */

	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save

	/* Stack set up, execute interrupt handler */
	/* R9 = HTID, r8=SSR, r7 = thread context */
	{
		r0 = and(r8,#0x1f)
		r1 = r7		// thread context
		r2 = r9
	}

#if __QDSP6_ARCH__ >= 4
	p0 = cmp.eq(r0,#31)
	r4 = VID

#if ARCHV == 4
	/* Work around V4SS errata */
	{
		r10 = memw(H2K_GP+#KG_l2_int_base)
		r13:12 = bitsplit(r4,#5)
	}
	{
		r10 = add(r10,#0x180)
		r11 = add(r10,#0x100)
		r12 = lsl(#1,r12)
	}
	if (p0) memw(r10+r13<<#2) = r12
	if (p0) r12 = memw(r11+r13<<#2)
#endif

	if (p0) r0 = add(r4,#32)
	r4 = #MAX_INTERRUPTS-1
	r0 = minu(r4,r0)
	r5 = mux(p0,##0x80000000,#0)
	ciad(r5)
#endif

	r4 = add(H2K_GP,#KG_inthandlers)
	r4 = addasl(r4,r0,#3)
	r5:4 = memd(r4)
	r3 = r5				// param
	p0 = cmp.eq(r4,#0)

	/* Registers set up, call specified handler */

	if (!p0) call .Lint_jump_stub 	// will jump to r4

H2K_interrupt_restore:
	call H2K_preempt		// Try to take interrupt before returning
	/* Restore all state and return */
	jump H2K_context_restore_return

.Lint_jump_stub:
	memw(r1+#CONTEXT_continuation) = r31
	jumpr r4

	.global H2K_interrupt_restore
FUNC_END H2K_handle_int

FUNC_START H2K_interrupted_waitmode .text.core.interrupt .p2align 2
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
	r0 = and(r0,#0x1f)
#if __QDSP6_ARCH__ >= 4
	p0 = cmp.eq(r0,#31)
	r4 = VID
	if (p0) r0 = add(r4,#32)
	r4 = #MAX_INTERRUPTS-1
	r0 = minu(r4,r0)
	r5 = mux(p0,##0x80000000,#0)
	ciad(r5)
#endif
	r1 = #0		// thread context == NULL
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)
	r4 = add(H2K_GP,#KG_inthandlers)
	r4 = addasl(r4,r0,#3)
	r5:4 = memd(r4)
	r3 = r5
	p0 = cmp.eq(r4,#0)
	/* Registers set up, call specified handler */
	if (!p0) callr r4
	r0 = #0
	r1 = #0
	k0lock
	jump H2K_switch

FUNC_END H2K_interrupted_waitmode

FUNC_START H2K_interrupted_preempt .text.core.interrupt .p2align 2
	/* We were at the preemption point, so we know a few things:
	 * r0 has the real value for SGP.  But we've already swapped
	 * it back.  
	 * 
	 * H2K_GP has the H2K_kg
	 * 
	 * There's a kernel stack pointer in r29, but it's not necessarily
	 * the stack base pointer.  We'll just re-setup the stack.
	 * 
	 * So, basically, this is the simplest case.  Most of the stuff
	 * we need is already available, and the biggest work item is to
	 * get the interrupt number, pointer, and jump to it.
	 * 
	 * Also restore ELR 
	 */
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
	r0 = and(r0,#0x1f)
#if __QDSP6_ARCH__ >= 4
	p0 = cmp.eq(r0,#31)
	r4 = VID
	if (p0) r0 = add(r4,#32)
	r4 = #MAX_INTERRUPTS-1
	r0 = minu(r4,r0)
	r5 = mux(p0,##0x80000000,#0)
	ciad(r5)
#endif
	r1 = sgp	// thread context
	r18 = r1
	r11 = memw(r1+#CONTEXT_elr)
	elr = r11	// restore background task ELR, clobbered by interrupt
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)
	r4 = add(H2K_GP,#KG_inthandlers)
	r4 = addasl(r4,r0,#3)
	r5:4 = memd(r4)
	r3 = r5
	p0 = cmp.eq(r4,#0)
	/* Registers set up, call specified handler */
	if (!p0) callr r4
	/* OK, get continuation and jump to it.  Restore r1:0 */
	r18 = sgp
	r31 = memw(r18+#CONTEXT_continuation)
	r1:0 = memd(r18+#CONTEXT_r0100)
	jumpr r31
FUNC_END H2K_interrupted_preempt


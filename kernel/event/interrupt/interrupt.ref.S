/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <trace_asm_macro.h>
#include <trace_constants.h>

#define TMP r31

#define INTNO r16

#define TRACEDATA r13:12
#define TRACEDATAHI r13
#define TRACEDATALO r12

#define INTINFO r11:10
#define INTINFO_VAL r11
#define INTINFO_FN r10

#define HTNUM r9
#define SSRVAL r8
#define ME r7
#define KG_INTHANDLERS r6
#define ENTRIES_BUFADDR r5:4
#define ENTRIES r5
#define BUFADDR r4

#define CIADVAL r2
#define VIDVAL r1

#define INT_SGP r0		// MUST BE R0

#define ISL2INT p0
#define TRACEPRED p1

FUNC_START H2K_handle_int .text.core.interrupt .p2align 2
	/* If SGP is null, we've interrupted waitmode. */
	crswap(INT_SGP,sgp)
	if (INT_SGP==#0) jump:t H2K_interrupted_waitmode

	/* If SGP is 0x00000001, we've interrupted the preemption point */
	INT_SGP = and(INT_SGP,#-2)
	if (INT_SGP == #0) jump:nt H2K_interrupted_preempt

	/* Save off all context */
	/* Optimization Note:
	 * First, we should (maybe) dcfetch r0 and r0+32 
	 * Next, we should add 64 to r0 and dczeroa
	 * Then continue to dczeroa the rest of the cache lines
	 * Finally, we should reset r0 back to the original point 
	 * so the context offset macros work. 
	 */

	memd(INT_SGP+#CONTEXT_r3130) = r31:30
	call H2K_context_save

	/* Stack set up, execute interrupt handler */
	/* R9 = HTID, r8=SSR, r7 = thread context */
	INTNO = and(SSRVAL,#0x1f)

#if ARCHV >= 4
	ISL2INT = cmp.eq(INTNO,#31)
	VIDVAL = VID

#if ARCHV == 4
	/* Work around V4SS errata */
#define BITSHI r15
#define BITSLO r14
#define BITS r15:14
#define L2BASE2 r5
#define L2BASE r4
	{
		L2BASE = memw(H2K_GP+#KG_l2_int_base)
		BITS = bitsplit(VIDVAL,#5)
	}
	{
		L2BASE = add(L2BASE,#0x180)
		L2BASE2 = add(L2BASE,#0x100)
		BITSLO = lsl(#1,BITSLO)
	}
	if (ISL2INT) memw(L2BASE+BITSHI<<#2) = BITSLO	// clear enable
	if (ISL2INT) L2BASE = memw(L2BASE2+BITSHI<<#2)	// sync
#undef BITSHI
#undef BITSLO
#undef BITS
#undef L2BASE 
#undef L2BASE2
#endif

	if (ISL2INT) INTNO = add(VIDVAL,#32)
	CIADVAL = mux(ISL2INT,##0x80000000,#0)
	ciad(CIADVAL)
#endif

	TMP = #H2K_TRACE_INTERRUPT
	TMP |= asl(INTNO,#8)
	TRACEDATAHI = HTNUM
	TRACEDATALO = pcyclelo
	ASM_INLINE_TRACE(TMP,TRACEDATA,TRACEDATAHI,TRACEDATALO,ENTRIES_BUFADDR,
		ENTRIES,BUFADDR,TRACEPRED,INTINFO_VAL,INTINFO_FN,VIDVAL,,)
	dccleana(BUFADDR)

	KG_INTHANDLERS = add(H2K_GP,#KG_inthandlers)
	KG_INTHANDLERS = addasl(KG_INTHANDLERS,INTNO,#3)
	INTINFO = memd(KG_INTHANDLERS)

	/* Registers set up, call specified handler */

	call .Lint_jump_stub 	// will jump to r4

H2K_interrupt_restore:
	call H2K_preempt		// Try to take interrupt before returning
	/* Restore all state and return */
	jump H2K_context_restore_return

.Lint_jump_stub:
	memw(ME+#CONTEXT_continuation) = r31
	r3:2 = combine(INTINFO_VAL,HTNUM)
	r1:0 = combine(ME,INTNO)
	jumpr INTINFO_FN

	.global H2K_interrupt_restore
FUNC_END H2K_handle_int

FUNC_START H2K_interrupted_waitmode .text.core.interrupt .p2align 2
#if ARCHV <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	HTNUM = ssr
	HTNUM = extractu(HTNUM,#3,#19)
	TMP = #KERNEL_STACK_SIZE
	r29 += mpyi(HTNUM,TMP)
#else
	HTNUM = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(HTNUM,r29))
#endif
	
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)

	/* Stack set up, execute interrupt handler */
	SSRVAL = ssr
	INTNO = and(SSRVAL,#0x1f)

	TMP = #H2K_TRACE_INTERRUPT
	TMP |= asl(INTNO,#8)
	TRACEDATAHI = HTNUM
	TRACEDATALO = pcyclelo
	ASM_INLINE_TRACE(TMP,TRACEDATA,TRACEDATAHI,TRACEDATALO,ENTRIES_BUFADDR,
		ENTRIES,BUFADDR,TRACEPRED,INTINFO_VAL,INTINFO_FN,ME,,)
	dccleana(BUFADDR)

#if ARCHV >= 4
	ISL2INT = cmp.eq(INTNO,#31)
	VIDVAL = VID
	if (ISL2INT) INTNO = add(VIDVAL,#32)
	CIADVAL = mux(ISL2INT,##0x80000000,#0)
	ciad(CIADVAL)
#endif
	r1 = #0		// thread context == NULL
	KG_INTHANDLERS = add(H2K_GP,#KG_inthandlers)
	KG_INTHANDLERS = addasl(KG_INTHANDLERS,INTNO,#3)
	INTINFO = memd(KG_INTHANDLERS)
	/* Registers set up, call specified handler */
	r3:2 = combine(INTINFO_VAL,HTNUM)
	r1:0 = combine(#0,INTNO)
	callr INTINFO_FN
	r1:0 = #0
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
#if ARCHV <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	HTNUM = ssr
	HTNUM = extractu(HTNUM,#3,#19)
	TMP = #KERNEL_STACK_SIZE
	r29 += mpyi(HTNUM,TMP)
#else
	HTNUM = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(HTNUM,r29))
#endif
	/* Stack set up, execute interrupt handler */
	SSRVAL = ssr
	INTNO = and(SSRVAL,#0x1f)
#if ARCHV >= 4
	ISL2INT = cmp.eq(INTNO,#31)
	VIDVAL = VID
	if (ISL2INT) INTNO = add(VIDVAL,#32)
	CIADVAL = mux(ISL2INT,##0x80000000,#0)
	ciad(CIADVAL)
#endif
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)

	TMP = #H2K_TRACE_INTERRUPT
	TMP |= asl(INTNO,#8)
	TRACEDATAHI = HTNUM
	TRACEDATALO = pcyclelo
	ASM_INLINE_TRACE(TMP,TRACEDATA,TRACEDATAHI,TRACEDATALO,ENTRIES_BUFADDR,
		ENTRIES,BUFADDR,TRACEPRED,INTINFO_VAL,INTINFO_FN,ME,,)
	dccleana(BUFADDR)

	ME = sgp0	// thread context
	TMP = memw(ME+#CONTEXT_elr)
	elr = TMP	// restore background task ELR, clobbered by interrupt
	KG_INTHANDLERS = add(H2K_GP,#KG_inthandlers)
	KG_INTHANDLERS = addasl(KG_INTHANDLERS,INTNO,#3)
	INTINFO = memd(KG_INTHANDLERS)
	/* Registers set up, call specified handler */
	r3:2 = combine(INTINFO_VAL,HTNUM)
	r1:0 = combine(ME,INTNO)
	callr INTINFO_FN
	/* OK, get continuation and jump to it.  Restore r1:0 */
	ME = sgp0
	r31 = memw(ME+#CONTEXT_continuation)
	r1:0 = memd(ME+#CONTEXT_r0100)
	jumpr r31
FUNC_END H2K_interrupted_preempt


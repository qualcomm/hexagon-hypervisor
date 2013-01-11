/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <trace_constants.h>

#define ELRTMP r31
#define PCYCLO r30		// must be r30!!

#define INTNO r16

#define ENTRIES r15
#define BUFADDR r14
#define ENTRIES_BUFADDR r15:14

#define OLDIDX r13
#define NEWIDX r12

#define INTINFO r11:10
#define INTINFO_VAL r11
#define INTINFO_FN r10

#define HTNUM r9
#define SSRVAL r8
#define ME r7
#define KG_INTHANDLERS r6

#define TRACEDATAHI r5
#define TRACEDATALO r4
#define TRACEDATA r5:4

#define IDXADDR r3
#define CIADVAL r2
#define VIDVAL r1
#define INT_SGP r0		// MUST BE R0

#define TRACEPRED p1
#define ISL2INT p0
#define ISPREEMPT p2

FUNC_START H2K_handle_int .text.core.interrupt .p2align 2
	/* Optimization Note:
	 * First, we should (maybe) dcfetch r0+32 
	 * Next, we should add 64 to r0 and dczeroa
	 * Then continue to dczeroa the rest of the cache lines
	 * Finally, we should reset r0 back to the original point 
	 * so the context offset macros work. 
	 */
	{
		crswap(INT_SGP,sgp0)
		if (!cmp.gtu(INT_SGP.new,#1)) jump:t H2K_interrupted_waitmode_or_preempt
	}
	{
		call H2K_context_save
		memd(r0+#CONTEXT_r3130) = r31:30
		PCYCLO = PCYCLELO
	}
	/* R7 = context, r8 = SSR, r9 = HTID */
	{
		INTNO = and(SSRVAL,#0x1f)  // extract interrupt number
		PCYCLO = insert(HTNUM,#4,#0)
		VIDVAL = VID
		IDXADDR = add(H2K_GP,#KG_trace_info_index)
	}
	{
		ISL2INT = cmp.eq(INTNO,#31)
		if (ISL2INT.new) INTNO = add(VIDVAL,#32)
		TRACEDATA = combine(#H2K_TRACE_INTERRUPT,PCYCLO)
		ENTRIES_BUFADDR = memd(H2K_GP+#KG_trace_info_entries_buf)
	}
1:
	{
		OLDIDX = memw_locked(IDXADDR)
		NEWIDX = add(ENTRIES,#-1)
		TRACEDATAHI |= asl(INTNO,#8)
	}
	{
		TRACEPRED = cmp.eq(NEWIDX,OLDIDX)
		if (!TRACEPRED.new) NEWIDX = add(OLDIDX,#1)
		if (TRACEPRED.new) NEWIDX = #0
		KG_INTHANDLERS = add(H2K_GP,#KG_inthandlers)
	}
	{
		memw_locked(IDXADDR,TRACEPRED) = NEWIDX
		BUFADDR = addasl(BUFADDR,OLDIDX,#3)
		CIADVAL = mux(ISL2INT,##0x80000000,#0)
	}
	{
		if (!TRACEPRED) jump 1b
		if (TRACEPRED) memd(BUFADDR) = TRACEDATA
		INTINFO = memd(KG_INTHANDLERS+INTNO<<#3)
	}
	{
		dccleana(BUFADDR)
		call .Lint_jump_stub
	}

	.falign
H2K_interrupt_restore:
	/* Optimization Note:
	 * First, we should dcfetch several lines ahead of
	 * where we load in order to prevent cache misses
	 * in the case of L1 miss/L2 hits.
	 */
	/* Restore all state and return */
	/* Can I optimize this to jump and set R31 to context_restore_return? */
	call H2K_preempt
	call H2K_context_restore_return
	.global H2K_interrupt_restore

	.falign
.Lint_jump_stub:
	/* Registers set up, call specified handler */
	{
		ciad(CIADVAL)
	}
	{
		r3:2 = combine(INTINFO_VAL,HTNUM)
		r1:0 = combine(ME,INTNO)
		memw(ME+#CONTEXT_continuation) = r31
		jumpr INTINFO_FN
	}

FUNC_END H2K_handle_int

FUNC_START H2K_interrupted_waitmode_or_preempt .text.core.interrupt .p2align 5
	{
		SSRVAL = ssr
		H2K_GP = ##H2K_kg		// unnecessary if just preempt, but we're sharing code
		ISPREEMPT = cmp.eq(INT_SGP,#1)
	}
	{
		// INT_SGP = r0		// save what was in SGP at interrupt time (0 or 1)
		// Note that we defined INT_SGP as r0 above
		r29 = #KERNEL_STACK_SIZE
		KG_INTHANDLERS = add(H2K_GP,#KG_inthandlers)	// inthandler pointer
		INTNO = and(SSRVAL,#0x1f)  		// extract interrupt number
		VIDVAL = VID			// vector ID if L2 interrupt
	}
	{
		ISL2INT = cmp.eq(INTNO,#31)		// L2 interrupt?
		if (ISL2INT.new) INTNO = add(VIDVAL,#32)	// Yes, use VID + offset
		IDXADDR = add(H2K_GP,#KG_trace_info_index)
		ME = sgp			// scheduled task or garbage
	}
	{
		ENTRIES_BUFADDR = memd(H2K_GP+#KG_trace_info_entries_buf)
		HTNUM = htid
		if (!ISPREEMPT) ME = #0		// 0 or SGP, was multiply which was fun
	}
	{
		TRACEDATAHI = #H2K_TRACE_INTERRUPT
		INTINFO = memd(KG_INTHANDLERS+INTNO<<#3)	// compute interrupt handler
		TRACEDATALO = PCYCLELO
	}
1:
	{
		OLDIDX = memw_locked(IDXADDR)
		NEWIDX = add(ENTRIES,#-1)
		TRACEDATAHI |= asl(INTNO,#8)
		TRACEDATALO = insert(HTNUM,#4,#0)
	}
	{
		TRACEPRED = cmp.eq(NEWIDX,OLDIDX)
		if (!TRACEPRED.new) NEWIDX = add(OLDIDX,#1)
		if (TRACEPRED.new) NEWIDX = #0
		SGP0 = ME
	}
	{
		memw_locked(IDXADDR,TRACEPRED) = NEWIDX
		BUFADDR = addasl(BUFADDR,OLDIDX,#3)
		CIADVAL = mux(ISL2INT,##0x80000000,#0)	// Value to CIAD if L2 interrupt
	}
	{
		if (!TRACEPRED) jump 1b
		if (TRACEPRED) memd(BUFADDR) = TRACEDATA
		if (ISPREEMPT) ELRTMP = memw(ME+#CONTEXT_elr)
	}
	{
		ciad(CIADVAL)			// Re-enable L2 controller interrupt if L2 int
		r29 = add(##H2K_stacks,mpyi(HTNUM,r29))
	}
	elr = ELRTMP
	{
		callr INTINFO_FN
		r1:0 = combine(ME,INTNO)
		r3:2 = combine(INTINFO_VAL,HTNUM)
		dccleana(BUFADDR)
	}
	{
		ME = sgp
		if (cmp.eq(ME.new,#0)) jump:t 1f
	}
	{
		r31 = memw(ME+#CONTEXT_continuation)
		r1:0 = memd(ME+#CONTEXT_r0100)
	}
	{
		jumpr r31
	}
1:						// We were idle, go back to sleep
	{
		k0lock
	}
	{
		r1:0 = #0
		jump H2K_switch
	}
FUNC_END H2K_interrupted_waitmode_or_preempt


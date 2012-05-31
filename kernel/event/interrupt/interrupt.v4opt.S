/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_handle_int .text.core.interrupt .p2align 2
	/* Optimization Note:
	 * First, we should (maybe) dcfetch r0+32 
	 * Next, we should add 64 to r0 and dczeroa
	 * Then continue to dczeroa the rest of the cache lines
	 * Finally, we should reset r0 back to the original point 
	 * so the context offset macros work. 
	 */
	{
		crswap(r0,sgp0)
		if (!cmp.gtu(r0.new,#1)) jump:t H2K_interrupted_waitmode_or_preempt
	}
	{
		call H2K_context_save
		memd(r0+#CONTEXT_r3130) = r31:30
	}
	/* R7 = context, r8 = SSR, r9 = HTID */
	{
		r13 = add(r16,#KG_inthandlers)
		r4 = and(r8,#0x1f)  // extract interrupt number
		r6 = VID
		r5 = #0
	}
	{
		p0 = cmp.eq(r4,#31)
		if (p0.new) r5 = add(r5,##0x80000000)
		if (p0.new) r4 = add(r6,#32)
	}
	{
		r1:0 = combine(r7,r4)
		r3 = memw(r13+r4<<#3)
		r2 = r9
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
	call H2K_preempt
	call H2K_context_restore_return
	.global H2K_interrupt_restore

.Lint_jump_stub:
	/* Registers set up, call specified handler */
	{
		ciad(r5)
	}
	{
		memw(r7+#CONTEXT_continuation) = r31
		jumpr r3
	}

FUNC_END H2K_handle_int

#if 0
FUNC_START H2K_interrupted_waitmode .text.core.interrupt .p2align 2
	{
		r0 = ssr
		r29 = #KERNEL_STACK_SIZE
		r16 = ##H2K_kg
	}
	{
		r0 = and(r0,#0x1f)  // extract interrupt number
		r1 = #0  // thread context == NULL
		r4 = add(r16,#KG_inthandlers)
		r5 = VID
	}
	{
		p0 = cmp.eq(r0,#31)
		r2 = htid
		if (p0.new) r0 = add(r5,#32)
	}
	/* Registers set up, call specified handler */
	{
		r3 = memw(r4+r0<<#3)  // compute interrupt handler
		r5 = mux(p0,##0x80000000,#0)
	}
	{
		r29 = add(##H2K_stacks,mpyi(r2,r29))
		ciad(r5)
	}
	{
		p0 = cmp.eq(r3,#0)
		if (!p0.new) jumpr:t r3
		r31 = ##1f
	}
1:
	{
		k0lock
	}
	{
		r1:0 = #0
		jump H2K_switch
	}
FUNC_END H2K_interrupted_waitmode

FUNC_START H2K_interrupted_preempt .text.core.interrupt .p2align 2
	{
		r0 = ssr
		r29 = #KERNEL_STACK_SIZE
		r4 = add(r16,#KG_inthandlers)
	}
	{
		r0 = and(r0,#0x1f)  // extract interrupt number
		r1 = sgp  // thread context == NULL	XXXXX
		r4 = add(r16,#KG_inthandlers)
		r5 = VID
	}
	{
		p0 = cmp.eq(r0,#31)
		r2 = htid
		if (p0.new) r0 = add(r5,#32)
	}
	/* Registers set up, call specified handler */
	{
		r3 = memw(r4+r0<<#3)  // compute interrupt handler
		r5 = mux(p0,##0x80000000,#0)
	}
	{
		r29 = add(##H2K_stacks,mpyi(r2,r29))
		ciad(r5)
	}
	{
		p0 = cmp.eq(r3,#0)
		if (!p0.new) jumpr:t r3
		r31 = ##1f
	}
1:
	{
		r0 = sgp
		if (cmp.eq(r0.new,#0)) jump:nt 2f
	}
	{
		r31 = memw(r0+#CONTEXT_continuation)
		r1:0 = memd(r0+#CONTEXT_r0100)
	}
	jumpr r31
2:
	{
		k0lock
	}
	{
		r1:0 = #0
		jump H2K_switch
	}
FUNC_END H2K_interrupted_preempt
#endif

FUNC_START H2K_interrupted_waitmode_or_preempt .text.core.interrupt .p2align 5
	{
		r2 = ssr
		r29 = #KERNEL_STACK_SIZE
		r16 = ##H2K_kg		// unnecessary if just preempt, but we're sharing code
		r17 = r0		// save what was in SGP at interrupt time (0 or 1)
					// allowed by duplex
	}
	{
		r2 = and(r2,#0x1f)  		// extract interrupt number
		r4 = add(r16,#KG_inthandlers)	// inthandler pointer
		r5 = VID			// vector ID if L2 interrupt
	}
	{
		p0 = cmp.eq(r2,#31)		// L2 interrupt?
		if (p0.new) r0 = add(r5,#32)	// Yes, use VID + offset
		if (!p0.new) r0 = r2		// No, just use L1 value
		r1 = sgp			// scheduled task or garbage
	}
	/* Registers set up, call specified handler */
	{
		r3 = memw(r4+r0<<#3)  		// compute interrupt handler
		r5 = mux(p0,##0x80000000,#0)	// Value to CIAD if L2 interrupt
		r2 = htid
	}
	{
		r29 = add(##H2K_stacks,mpyi(r2,r29))
		p0 = cmp.eq(r3,#0)		// Is handler NULL?
		ciad(r5)			// Re-enable L2 controller interrupt if L2 int
	}
	{
		r1 = mpyi(r1,r17)		// 0 or 1 times SGP == 0 or SGP
		if (!p0) callr r3		// If non-null, call int handler
		r18 = r1			// save SGP ptr
	}
	{
		p0 = cmp.eq(r17,#0)		// If input was 0 (idle), go back to sleep
		if (p0.new) jump:nt 1f		
		if (!p0.new) r31 = memw(r18+#CONTEXT_continuation)	// Get continuation
		if (!p0.new) r1:0 = memd(r18+#CONTEXT_r0100)		// Get return val
	}
	jumpr r31
1:						// We were idle, go back to sleep
	{
		k0lock
	}
	{
		r1:0 = #0
		jump H2K_switch
	}
FUNC_END H2K_interrupted_waitmode_or_preempt


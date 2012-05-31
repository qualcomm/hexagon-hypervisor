/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

	.section .text.core.interrupt,"ax",@progbits
	.p2align 5
.Lint_jump_stub:
	{
		memw(r1+#CONTEXT_continuation) = r31
		jumpr r4
	}
FUNC_START H2K_handle_int .text.core.interrupt .p2align 2
	/* If SGP is null, we've interrupted waitmode. */
	crswap(r0,sgp)
	{
		dcfetch(r0)
		if (r0==#0) jump:t H2K_interrupted_waitmode
		r0 = and(r0,#-2)
	}
	{
		if (r0==#0) jump:nt H2K_interrupted_preempt
	}
	{
		memd(r0+#CONTEXT_r3130) = r31:30
		call H2K_context_save
	}
	/* TNUM: r9, SSR: r8, SGP: r7 */
	{
		r0 = and(r8,#MAX_INTERRUPTS-1)
		/* ??? memw(rXX+#CONTEXT_continuation) = r31 */
		r4 = add(r16,#KG_inthandlers)
	}
	{
		r4 = addasl(r4,r0,#3)
		r2 = r9
	}
	{
		r4 = memw(r4)
		r1:0 = combine(r7,r0)
		call .Lint_jump_stub
	}
	// 30 cycles ideal (including stub)
H2K_interrupt_restore:
	/* Restore all state and return */
	call H2K_preempt
	jump H2K_context_restore_return
	.global H2K_interrupt_restore
FUNC_END H2K_handle_int

FUNC_START H2K_interrupted_waitmode .text.core.interrupt .falign
	{
		r29.h = #hi(H2K_stacks)
		r0 = ssr
		r3 = #KERNEL_STACK_SIZE
		r4.h = #hi(H2K_kg+KG_inthandlers)
	}
	{
		r29.l = #lo(H2K_stacks)
		r2 = extractu(r0,#3,#19)
		r4.l = #lo(H2K_kg+KG_inthandlers)
		r0 = and(r0,#MAX_INTERRUPTS-1)
	}
	{
		r29 += mpyi(r2,r3)
		r1 = #0
		r16 = add(r4,#-KG_inthandlers)
		r4 = addasl(r4,r0,#3)
	}
	{
		r4 = memw(r4)
	}
	{
		callr r4
	}
	k0lock
	{
		r1:0 = combine(#0,#0)
		jump H2K_switch
	}
FUNC_END H2K_interrupted_waitmode

FUNC_START H2K_interrupted_preempt .text.core.interrupt .falign
	{
		r29.h = #hi(H2K_stacks)
		r0 = ssr
		r3 = #KERNEL_STACK_SIZE
		r4 = add(r16,#KG_inthandlers)
	}
	{
		r29.l = #lo(H2K_stacks)
		r2 = extractu(r0,#3,#19)
		r0 = and(r0,#MAX_INTERRUPTS-1)
	}
	{
		r18 = sgp
		r4 = addasl(r4,r0,#3)
	}
	{
		r29 += mpyi(r2,r3)
		r4 = memw(r4)
		r1 = r18
	}
	{
		callr r4				// call interrupt handler
	}
	{
		r31 = memw(r18+#CONTEXT_continuation)
		r1:0 = memd(r18+#CONTEXT_r0100)
	}
	jumpr r31					// call continuation if returning
FUNC_END H2K_interrupted_preempt


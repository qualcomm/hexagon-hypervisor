/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

	.section .text.event.interrupt,"ax",@progbits
	.p2align 5
.Lint_jump_stub:
	{
		memw(r1+#CONTEXT_continuation) = r31
		jumpr r4
	}
FUNC_START H2K_handle_int .text.event.interrupt .p2align 2
	/* If SGP is null, we've interrupted waitmode. */
	crswap(r0,sgp)
	{
		dcfetch(r0)
		if (r0==#0) jump:t H2K_interrupted_waitmode
		r0 = add(r0,#160)
	}
	{
		memd(r0+#CONTEXT_r3130-160) = r31:30
		r31.h = #hi(H2K_fastint_contexts)
	}
	{
		dczeroa(r0)				// 160
		r31.l = #lo(H2K_fastint_contexts)
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)				// 192
		r30 = sub(r0,r31)
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)				// 224
		r30 = asr(r30,#5)
		r0 = add(r0,#-128)
	}
	{
		dczeroa(r0)				// 96
		p0 = cmp.gtu(r30,#(((FASTINT_CONTEXT_SIZE*MAX_HTHREADS)/32)-1))
		r30 = p3:0
	}
	{
		memd(r0+#CONTEXT_r2928-96) = r29:28
		if (!p0) jump H2K_interrupted_fastint_check
		r31 = usr
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)				// 128
		r0 = add(r0,#-128)
		r29 = ugp
	}
	{
		memd(r0+#CONTEXT_r1514) = r15:14
		r28 = gp
	}
	{
		memd(r0+#CONTEXT_r1312) = r13:12
		r15 = lc0
	}
	{
		memd(r0+#CONTEXT_r1110) = r11:10
		r14 = sa0
	}
	{
		memd(r0+#CONTEXT_r0908) = r9:8
		r13 = lc1
	}
	{
		memd(r0+#CONTEXT_r0706) = r7:6
		r12 = sa1
	}
	{
		memd(r0+#CONTEXT_r0504) = r5:4
		r11 = m1
	}
	{
		memd(r0+#CONTEXT_r0302) = r3:2
		r10 = m0
	}
	{
		memd(r0+#CONTEXT_lc0sa0) = r15:14
		r9 = ssr
	}
	{
		memd(r0+#CONTEXT_lc1sa1) = r13:12
		r8 = elr
	}
	{
		memd(r0+#CONTEXT_m1m0) = r11:10
		r7 = and(r9,#MAX_INTERRUPTS-1)
	}
	{
		memd(r0+#CONTEXT_sr_preds) = r31:30
		r2 = extractu(r9,#3,#19)		// hw tnum
	}
	{
		memd(r0+#CONTEXT_ssrelr) = r9:8
	}
	{
		memd(r0+#CONTEXT_ugpgp) = r29:28
	}
	{
		memd(r0+#CONTEXT_r1716) = r17:16
		r16.h = #hi(H2K_kg)
		r29.h = #hi(H2K_stacks)
	}
	{
		memd(r0+#CONTEXT_r1918) = r19:18
		r16.l = #lo(H2K_kg)
		r29.l = #lo(H2K_stacks)
	}
	{
		memd(r0+#CONTEXT_r2120) = r21:20
		r4 = add(r16,#KG_inthandlers)
		r3 = #KERNEL_STACK_SIZE
	}
	{
		memd(r0+#CONTEXT_r2322) = r23:22
		r4 = addasl(r4,r7,#2)
		r29 += mpyi(r2,r3)
	}
	{
		memd(r0+#CONTEXT_r2524) = r25:24
		r4 = memw(r4)
		r21 = r0
	}
	{
		memd(r0+#CONTEXT_r2726) = r27:26
		crswap(r0,sgp)
	}
	{
		memd(r21+#CONTEXT_r0100) = r1:0
		r1:0 = combine(r21,r7)
		call .Lint_jump_stub
	}
	// 30 cycles ideal (including stub)
H2K_interrupt_restore:
	/* Restore all state and return */
	r0 = sgp
	{
		r3:2 = memd(r0+#CONTEXT_ssrelr)
		dcfetch(r0+#CONTEXT_lc1sa1)
	}
	{
		r31:30 = memd(r0+#CONTEXT_r3130)
		dcfetch(r0+#CONTEXT_r0706)
		elr = r2
	}
	{
		dcfetch(r0+#CONTEXT_r1514)
		ssr = r3
	}
	{
		dcfetch(r0+#CONTEXT_r1918)
		r25:24 = memd(r0+#CONTEXT_m1m0)
	}
	{
		r29:28 = memd(r0+#CONTEXT_r2928)
		dcfetch(r0+#CONTEXT_r2726)
		m0 = r24
	}
	{
		r27:26 = memd(r0+#CONTEXT_lc1sa1)
		r21:20 = memd(r0+#CONTEXT_sr_preds)
		m1 = r25
	}
	{
		r23:22 = memd(r0+#CONTEXT_lc0sa0)
		r3:2 = memd(r0+#CONTEXT_r0302)
		sa1 = r26
	}
	{
		r5:4 = memd(r0+#CONTEXT_r0504)
		r7:6 = memd(r0+#CONTEXT_r0706)
		lc1 = r27
	}
	{
		r9:8 = memd(r0+#CONTEXT_r0908)
		r11:10 = memd(r0+#CONTEXT_r1110)
		p3:0 = r20
	}
	{
		r13:12 = memd(r0+#CONTEXT_r1312)
		r15:14 = memd(r0+#CONTEXT_r1514)
		usr = r21
	}
	{
		r27:26 = memd(r0+#CONTEXT_ugpgp)
		r17:16 = memd(r0+#CONTEXT_r1716)
		sa0 = r22
	}
	{
		r19:18 = memd(r0+#CONTEXT_r1918)
		lc0 = r23
	}
	{
		r21:20 = memd(r0+#CONTEXT_r2120)
		r23:22 = memd(r0+#CONTEXT_r2322)
		gp = r26
	}
	{
		r25:24 = memd(r0+#CONTEXT_r2524)
		r27:26 = memd(r0+#CONTEXT_r2726)
		ugp = r27
	}
	{
		r1:0 = memd(r0+#CONTEXT_r0100)
		rte
	}
	// restore: 16 cycles (ideal)
	.global H2K_interrupt_restore
FUNC_END H2K_handle_int

FUNC_START H2K_interrupted_waitmode .text.event.interrupt .falign
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
		r4 = addasl(r4,r0,#2)
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


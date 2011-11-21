/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

/*
 * Context Save
 * R0/SGP swapped
 * R31:30 saved
 * Save r0-r29, all appropriate CRs
 */

FUNC_START H2K_context_save .text.core.context .p2align 2
	{
		memd(r0+#CONTEXT_r2928) = r29:28
	}
	{
		memd(r0+#CONTEXT_r2726) = r27:26
	}
	{
		memd(r0+#CONTEXT_r2524) = r25:24
		r25 = usr
	}
	{
		memd(r0+#CONTEXT_r2322) = r23:22
		r24 = p3:0
	}
	{
		memd(r0+#CONTEXT_r2120) = r21:20
		r23 = gp
	}
	{
		memd(r0+#CONTEXT_r1918) = r19:18
		r22 = ugp
	}
	{
		memd(r0+#CONTEXT_r1716) = r17:16
		r19 = lc0
	}
	{
		memd(r0+#CONTEXT_r1514) = r15:14
		r18 = sa0
	}
	{
		memd(r0+#CONTEXT_r1312) = r13:12
		r17 = lc1
	}
	{
		memd(r0+#CONTEXT_r1110) = r11:10
		r16 = sa1
	}
	{
		memd(r0+#CONTEXT_r0908) = r9:8
		r15 = m1
	}
	{
		memd(r0+#CONTEXT_r0706) = r7:6
		r14 = m0
	}
	{
		memd(r0+#CONTEXT_r0504) = r5:4
		r13 = ssr
	}
	{
		memd(r0+#CONTEXT_r0302) = r3:2
		r12 = elr
	}
	{
		r7 = r0
		crswap(r0,sgp)
	}
	{
		memd(r7+#CONTEXT_r0100) = r1:0
	}
	{
		memd(r7+#CONTEXT_sr_preds) = r25:24
		r29.h = #hi(H2K_stacks)
	}
	{
		memd(r7+#CONTEXT_gpugp) = r23:22
		r29.l = #lo(H2K_stacks)
	}
	{
		memd(r7+#CONTEXT_lc0sa0) = r19:18
		r28 = extractu(r13,#3,#19)
	}
	{
		memd(r7+#CONTEXT_lc1sa1) = r17:16
		r29 += mpyi(r28,r11)
		r16.h = #hi(H2K_kg)
	}
	{
		memd(r7+#CONTEXT_m1m0) = r15:14
		r16.l = #lo(H2K_kg)
	}
	{
		memd(r7+#CONTEXT_ssrelr) = r13:12
		callr r31
	}
FUNC_END H2K_context_save

FUNC_START H2K_context_restore_return .text.core.context .p2align 2
	r29 = sgp
	{
		r13:12 = memd(r29+#CONTEXT_ssrelr)
	}
	{
		r15:14 = memd(r29+#CONTEXT_m1m0)
		r17:16 = memd(r29+#CONTEXT_lc1sa1)
	}
		elr = r12
	{
		r19:18 = memd(r29+#CONTEXT_lc0sa0)
		r23:22 = memd(r29+#CONTEXT_gpugp)
		ssr = r13
	}
	{
		r25:24 = memd(r29+#CONTEXT_sr_preds)
		r13:12 = memd(r29+#CONTEXT_r1312)
		m0 = r14
	}
	{
		r1:0 = memd(r29+#CONTEXT_r0100)
		r15:14 = memd(r29+#CONTEXT_r1514)
		m1 = r15
	}
	{
		r31:30 = memd(r29+#CONTEXT_r3130)
		r21:20 = memd(r29+#CONTEXT_r2120)
		sa1 = r16
	}
	{
		r17:16 = memd(r29+#CONTEXT_r1716)
		lc1 = r17
	}
	{
		r3:2 = memd(r29+#CONTEXT_r0302)
		r5:4 = memd(r29+#CONTEXT_r0504)
		sa0 = r18
	}
	{
		r7:6 = memd(r29+#CONTEXT_r0706)
		r9:8 = memd(r29+#CONTEXT_r0908)
		lc0 = r19
	}
	{
		r11:10 = memd(r29+#CONTEXT_r1110)
		r19:18 = memd(r29+#CONTEXT_r1918)
		ugp = r22
	}
	{
		r27:26 = memd(r29+#CONTEXT_r2726)
		r23:22 = memd(r29+#CONTEXT_r2322)
		gp = r23
	}
	{
		p3:0 = r24
	}
	{
		r29:28 = memd(r29+#CONTEXT_r2928)
		r25:24 = memd(r29+#CONTEXT_r2524)
		usr = r25
	}
	rte
FUNC_END H2K_context_restore_return


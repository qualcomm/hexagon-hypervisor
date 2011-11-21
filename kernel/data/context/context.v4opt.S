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
 * Context restore returned in R31
 * Also set up Kernel Stack, KGP
 */

FUNC_START H2K_context_save .text.core.context .p2align 2
	{
		dcfetch(r0+#32)
		r0 = add(r0,#96)
	}
	{
		dczeroa(r0)		// 96
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)		// 128
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)		// 160
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)		// 192
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)		// 224
		r0 = add(r0,#-224)
		r30 = HTID
	}
	{
		memd(r0+#CONTEXT_r2928) = r29:28
		memd(r0+#CONTEXT_r2726) = r27:26
		r27:26 = s7:6			// ccr:ssr
	}
	{
		memd(r0+#CONTEXT_r2524) = r25:24
		memd(r0+#CONTEXT_r2322) = r23:22
		r23:22 = c11:10			// gp:ugp
	}
	{
		memd(r0+#CONTEXT_r2120) = r21:20
		memd(r0+#CONTEXT_r1918) = r19:18
		r21:20 = c13:12			// cs1:cs0
	}
	{
		memd(r0+#CONTEXT_r1716) = r17:16
		memd(r0+#CONTEXT_r1514) = r15:14
		r19:18 = c1:0			// lc0/sa0
	}
	{
		memd(r0+#CONTEXT_r1312) = r13:12
		memd(r0+#CONTEXT_r1110) = r11:10
		r17:16 = c3:2			// lc1/sa1
	}
	{
		memd(r0+#CONTEXT_r0908) = r9:8
		memd(r0+#CONTEXT_r0706) = r7:6
		r15:14 = c7:6			// m1:m0
	}
	{
		memd(r0+#CONTEXT_r0504) = r5:4
		memd(r0+#CONTEXT_r0302) = r3:2
		r7 = r0
		crswap(r0,sgp0)
	}
	{
		memd(r7+#CONTEXT_gpugp) = r23:22
		memd(r7+#CONTEXT_lc0sa0) = r19:18
		r28 = p3:0
	}
	{
		memd(r7+#CONTEXT_r0100) = r1:0
		memd(r7+#CONTEXT_lc1sa1) = r17:16
		r29 = usr
		r16.h = #hi(H2K_kg)
	}
	{
		memd(r7+#CONTEXT_m1m0) = r15:14
		memw(r7+#CONTEXT_ccr) = r27
		r27 = r26		// ssr
		r26 = elr
	}
	{
		memd(r7+#CONTEXT_sr_preds) = r29:28
		memd(r7+#CONTEXT_cs1cs0) = r21:20
		r21:20 = g3:2
		r16.l = #lo(H2K_kg)
	}
	{
		memd(r7+#CONTEXT_ssrelr) = r27:26
		memd(r7+#CONTEXT_gbadva_gosp) = r21:20
		r19:18 = g1:0
		r29 = #KERNEL_STACK_SIZE
	}
	{
		r29 = add(##H2K_stacks,mpyi(r30,r29))
		memd(r7+#CONTEXT_gssr_gelr) = r19:18
		callr r31
	}
FUNC_END H2K_context_save

FUNC_START H2K_context_restore_return .text.core.context .p2align 2
	/*
	 * EJP: note that some lines are likely to be in cache already:
	 * next/prev in first line, continuation in third line
	 */
	r27 = sgp
	{
		dcfetch(r27+#32)
		r29:28 = memd(r7+#CONTEXT_gssr_gelr)
	}
	{
		dcfetch(r27+#224)
		r13:12 = memd(r27+#CONTEXT_ssrelr)
		g1:0 = r29:28
	}
	{
		dcfetch(r27+#192)
		r29:28 = memd(r27+#CONTEXT_r2928)
		ssr = r13
	}
	{
		dcfetch(r27+#160)
		r31:30 = memd(r27+#CONTEXT_r3130)
	}
	elr = r12
	{
		dcfetch(r27+#128)
		r15:14 = memd(r27+#CONTEXT_gbadva_gosp)
	}
	{
		dcfetch(r27+#96)
		r8 = memw(r27+#CONTEXT_ccr)
		g3:2 = r15:14
	}
	{
		r21:20 = memd(r27+#CONTEXT_cs1cs0)
		r25:24 = memd(r27+#CONTEXT_sr_preds)
	}
	ccr = r8
	{
		r15:14 = memd(r27+#CONTEXT_m1m0)
		r17:16 = memd(r27+#CONTEXT_lc1sa1)
		c13:12 = r21:20		// cs1:0
	}
	{
		r19:18 = memd(r27+#CONTEXT_lc0sa0)
		r3:2 = memd(r27+#CONTEXT_r0302)
		c3:2 = r17:16		// lc1/sa1
	}
	{
		r5:4 = memd(r27+#CONTEXT_r0504)
		r7:6 = memd(r27+#CONTEXT_r0706)
		c7:6 = r15:14		// m1:m0
	}
	{
		r9:8 = memd(r27+#CONTEXT_r0908)
		r11:10 = memd(r27+#CONTEXT_r1110)
		c1:0 = r19:18		// lc0/sa0
	}
	{
		r13:12 = memd(r27+#CONTEXT_r1312)
		r15:14 = memd(r27+#CONTEXT_r1514)
		usr = r25
	}
	{
		r1:0 = memd(r27+#CONTEXT_r0100)
		r23:22 = memd(r27+#CONTEXT_gpugp)
		p3:0 = r24
	}
	{
		r17:16 = memd(r27+#CONTEXT_r1716)
		r19:18 = memd(r27+#CONTEXT_r1918)
		c11:10 = r23:22		// gp:ugp
	}
	{
		r21:20 = memd(r27+#CONTEXT_r2120)
		r23:22 = memd(r27+#CONTEXT_r2322)
	}
	{
		r25:24 = memd(r27+#CONTEXT_r2524)
		r27:26 = memd(r27+#CONTEXT_r2726)
	}
	rte
FUNC_END H2K_context_restore_return


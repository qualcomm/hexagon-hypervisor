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

FUNC_START H2K_context_save .text.core.context .p2align 5
	{
		memd(r0+#CONTEXT_r1918) = r19:18
		memd(r0+#CONTEXT_r2928) = r29:28
		r19:18 = c1:0			// lc0/sa0
		r29.h = #hi(H2K_stacks)
	}
	{
		memd(r0+#CONTEXT_r1110) = r11:10
		memd(r0+#CONTEXT_r0908) = r9:8
		r9:8 = s7:6			// ccr:ssr
		r29.l = #lo(H2K_stacks)
	}
	{
		memd(r0+#CONTEXT_lc0sa0) = r19:18
		memd(r0+#CONTEXT_r1716) = r17:16
		r19 = usr
		r28 = #KERNEL_STACK_SIZE
	}
	{
		memd(r0+#CONTEXT_r0706) = r7:6
		memd(r0+#CONTEXT_ccrssr) = r9:8
		r9 = HTID
		r7 = r0
	}
	{
		memd(r7+#CONTEXT_r0302) = r3:2
		memd(r7+#CONTEXT_r0504) = r5:4
		r29 += mpyi(r9,r28)
		r18 = p3:0
	}
	{
		memd(r7+#CONTEXT_r1514) = r15:14
		memd(r7+#CONTEXT_r1312) = r13:12
		crswap(r0,sgp0)
		r28.h = #hi(H2K_kg)
	}
	{
		memd(r7+#CONTEXT_r0100) = r1:0
		memd(r7+#CONTEXT_sr_preds) = r19:18
		callr r31
		r28.l = #lo(H2K_kg)
	}
FUNC_END H2K_context_save

FUNC_START H2K_context_restore_return .text.core.context .p2align 2
	/*
	 * EJP: note that some lines are likely to be in cache already:
	 * next/prev in first line, continuation in third line
	 */
	r16 = sgp0					// can save if sgp in known register
	{
		r13:12 = memd(r16+#CONTEXT_ccrssr)
		r15:14 = memd(r16+#CONTEXT_sr_preds)
	}
	{
		r29:28 = memd(r16+#CONTEXT_r2928)
		r31:30 = memd(r16+#CONTEXT_r3130)
		s7:6 = r13:12				// ccr:ssr
	}
	{
		r19:18 = memd(r16+#CONTEXT_lc0sa0)
		r3:2 = memd(r16+#CONTEXT_r0302)
	}
	{
		r5:4 = memd(r16+#CONTEXT_r0504)
		r7:6 = memd(r16+#CONTEXT_r0706)
		c1:0 = r19:18		// lc0/sa0
	}
	{
		r9:8 = memd(r16+#CONTEXT_r0908)
		r11:10 = memd(r16+#CONTEXT_r1110)
		usr = r15
	}
	{
		r1:0 = memd(r16+#CONTEXT_r0100)
		r19:18 = memd(r16+#CONTEXT_r1918)
		p3:0 = r14
	}
	{
		r15:14 = memd(r16+#CONTEXT_r1514)
		r13:12 = memd(r16+#CONTEXT_r1312)
	}
	{
		r17:16 = memd(r16+#CONTEXT_r1716)
	}
	rte						// simulator bug?  Should be able to schedule w/ above
FUNC_END H2K_context_restore_return


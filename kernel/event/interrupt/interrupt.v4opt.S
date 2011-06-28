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
	}
	/* If SGP is null, we've interrupted waitmode. */
	{
		if (r0==#0) jump:t H2K_interrupted_waitmode
		dcfetch(r0+#32)
		r0 = add(r0,#96)
	}
	{
		dczeroa(r0)  // 96
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)  // 128
		r0 = add(r0,#32)
	}
	{
		dczeroa(r0)  // 160
		r0 = add(r0,#32)
	}
	/* Save off all context */
	{
		memd(r0+#CONTEXT_r2726-192) = r27:26  // 96
		memd(r0+#CONTEXT_r2524-192) = r25:24
		r24 = sub(##H2K_fastint_contexts+192,r0)
	}
	{
		memd(r0+#CONTEXT_r2322-192) = r23:22
		memd(r0+#CONTEXT_r2120-192) = r21:20
		r26 = p3:0
		r24 = asr(r24,#5)
	}
	{
		dczeroa(r0)  // 192
		r0 = add(r0,#32)
		p0 = cmp.gt(r24,#(-((FASTINT_CONTEXT_SIZE*MAX_HTHREADS)/32)))
	}
	/* Check to see whether we interrupted a fast interrupt check */
	/* If so, vector off to H2K_interrupted_fastint_check */
	{
		memd(r0+#CONTEXT_r1918-224) = r19:18  // 128
		memd(r0+#CONTEXT_r1716-224) = r17:16
		r23:22 = c3:2  // lc1:sa1
		//if (p0) jump #H2K_interrupted_fastint_check
	}
	{
		memd(r0+#CONTEXT_r1514-224) = r15:14  // 160
		memd(r0+#CONTEXT_r1312-224) = r13:12
		r25:24 = c1:0  // lc0:sa0
		r12 = #KERNEL_STACK_SIZE
	}
	{
		dczeroa(r0)  // 224
		r0 = add(r0,#-160)
		r16 = ##H2K_kg
	}
	{
		dczeroa(r0)  // 64
		r0 = add(r0,#-64)
		r13 = add(r16,#KG_inthandlers)
	}
	{
		memd(r0+#CONTEXT_r1110) = r11:10
		memd(r0+#CONTEXT_r0908) = r9:8
		r27 = usr
	}
	{
		memd(r0+#CONTEXT_r0706) = r7:6  // 192
		memd(r0+#CONTEXT_r0504) = r5:4
		r21:20 = c7:6  // m1:m0
		r7 = r0
	}
	{
		memd(r7+#CONTEXT_r0302) = r3:2
		memd(r7+#CONTEXT_lc0sa0) = r25:24
		r19:18 = s7:6  // ccr:ssr
	}
	{
		memw(r7+#CONTEXT_ccr) = r19
		memd(r7+#CONTEXT_lc1sa1) = r23:22
		r19 = elr
		r4 = and(r18,#0x1f)  // extract interrupt number
	}
	{
		memd(r7+#CONTEXT_m1m0) = r21:20
		memd(r7+#CONTEXT_sr_preds) = r27:26
		r15:14 = c13:12  // cs1:cs0
		r19:18 = combine(r18,r19)  // swap ssr and elr
	}
	{
		memd(r7+#CONTEXT_cs1cs0) = r15:14
		memd(r7+#CONTEXT_r2928) = r29:28
		r11:10 = c11:10  // gp:ugp
	}
	{
		memd(r7+#CONTEXT_ugpgp) = r11:10
		memd(r7+#CONTEXT_r3130) = r31:30
		r2 = htid
		p0 = cmp.eq(r4,#31)
	}
	{
		memd(r7+#CONTEXT_ssrelr) = r19:18
		r29 = add(##H2K_stacks,mpyi(r2,r12))
		r6 = VID
	}
	{
		crswap(r0,sgp0)
		if (p0) r4 = add(r6,#32)
		r5 = mux(p0,##0x80000000,#0)
	}
	{
		memd(r7+#CONTEXT_r0100) = r1:0
		r1:0 = combine(r7,r4)
		r3 = memw(r13+r4<<#2)
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
	{
		r0 = sgp0
	}
	{
		dcfetch(r0+#224)
	}
	{
		dcfetch(r0+#192)
	}
	{
		dcfetch(r0+#64)
	}
	{
		dcfetch(r0+#32)
	}
	{
		dcfetch(r0+#128)
	}
	{
		dcfetch(r0+#160)
		r9:8 = memd(r0+#CONTEXT_cs1cs0)  //224
	}
	{
		dcfetch(r0+#96)
		r11:10 = memd(r0+#CONTEXT_sr_preds)
		c13:12 = r9:8  // cs1:cs0
	}
	{
		r13:12 = memd(r0+#CONTEXT_m1m0)
		r15:14 = memd(r0+#CONTEXT_lc1sa1)
		usr = r11
	}
	{
		r21:20 = memd(r0+#CONTEXT_lc0sa0)  // 192
		r3:2 = memd(r0+#CONTEXT_r0302)
		p3:0 = r10
	}
	{
		r5:4 = memd(r0+#CONTEXT_r0504)
		r7:6 = memd(r0+#CONTEXT_r0706)
		c7:6 = r13:12  // m1:m0
	}
	{
		r29:28 = memd(r0+#CONTEXT_r2928)  // 64
		r31:30 = memd(r0+#CONTEXT_r3130)
		c3:2 = r15:14  // lc1:sa1
	}
	{
		r23:22 = memd(r0+#CONTEXT_ssrelr)
		r25 = memw(r0+#CONTEXT_ccr)  // 32
		c1:0 = r21:20  // lc0:sa0
	}
	{
		r27:26 = memd(r0+#CONTEXT_ugpgp)  // 128
		r17:16 = memd(r0+#CONTEXT_r1716)
		r25:24 = combine(r25,r23)
	}
	{
		elr = r22  // AXOK
	}
	{
		s7:6 = r25:24  // ccr:ssr  AXOK
	}
	{
		r19:18 = memd(r0+#CONTEXT_r1918)
		r9:8 = memd(r0+#CONTEXT_r0908)  // 160
		c11:10 = r27:26  // gp:ugp
	}
	{
		r11:10 = memd(r0+#CONTEXT_r1110)
		r13:12 = memd(r0+#CONTEXT_r1312)
	}
	{
		r15:14 = memd(r0+#CONTEXT_r1514)
		r21:20 = memd(r0+#CONTEXT_r2120)  // 96
	}
	{
		r23:22 = memd(r0+#CONTEXT_r2322)
		r25:24 = memd(r0+#CONTEXT_r2524)
	}
	{
		r27:26 = memd(r0+#CONTEXT_r2726)
		r1:0 = memd(r0+#CONTEXT_r0100)
	}
	{
		rte  // ??? AXOK?
	}
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

FUNC_START H2K_interrupted_waitmode .text.core.interrupt .p2align 2
	{
		r0 = ssr
		r3 = #KERNEL_STACK_SIZE
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
		r3 = memw(r4+r0<<#2)  // compute interrupt handler
		r29 = add(##H2K_stacks,mpyi(r2,r3))
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

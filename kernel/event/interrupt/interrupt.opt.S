/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

FUNC_START BLASTK_handle_int .text.event.interrupt .p2align 2
	crswap(r0,sgp)
	{
		memd(r0+#CONTEXT_r3130) = r31:30
		r30.h = #HI(BLASTK_idle_context)
		r31.h = #HI(interrupted_waitmode_constants)
	}
	{
		memd(r0+#CONTEXT_r2928) = r29:28
		r30.l = #LO(BLASTK_idle_context)
		r31.l = #LO(interrupted_waitmode_constants)
		r28 = p3:0
	}
	{
		memd(r0+#CONTEXT_r2322) = r23:22
		p0 = cmp.eq(r0,r30)
		r23.h = #hi(BLASTK_fastint_contexts)
		r22 = add(r0,#((CONTEXT_r2120 + 31) & (-32)))
	}
	{
		dczeroa(r22)
		// slot 1 empty
		if (p0) jump BLASTK_interrupted_waitmode
		r23.l = #lo(BLASTK_fastint_contexts)
	}
	/* It breaks my heart to see these slots go to dczeroa's, but my 
	 * calculations are that it is better for performance if the data
	 * cache isn't there
	 * TBD: make sure r2322 is aligned to 32-byte boundary?
         */
	{
		memd(r0+#CONTEXT_r2120) = r21:20
		r21 = add(r0,#((CONTEXT_r2120 + 32 + 31) & (-32)))
		r23 = sub(r0,r23)
	}
	{
		dczeroa(r21)
		// slot 1 empty
		r21 = add(r0,#((CONTEXT_r2120 + 64 + 31) & (-32)))
		r23 = asr(r23,#THREAD_CONTEXT_TOTALSIZE_LOG2)
	}
	{
		dczeroa(r21)
		// slot 1 empty
		p0 = cmp.gtu(r23,#5)
		if (!p0.new) jump:nt blast_interrupted_fastint_check
	}
	{
		memd(r0+#CONTEXT_r1918) = r19:18
	}
	{
		memd(r0+#CONTEXT_r1716) = r17:16
		r29 = usr
	}
	{
		memd(r0+#CONTEXT_r1514) = r15:14
		r23 = ugp
	}
	{
		memd(r0+#CONTEXT_r1312) = r13:12
		r22 = gp
	}
	{
		memd(r0+#CONTEXT_r1110) = r11:10
		r21 = lc0
	}
	{
		memd(r0+#CONTEXT_r0908) = r9:8
		r20 = sa0
		r15 = r0
	}
	crswap(r0,sgp)
	r13 = ssr
	r12 = elr
	{
		memd(r15+#CONTEXT_r0706) = r7:6
		r11 = zxtb(r13)	// find CAUSE
		r19 = lc1
	}
    TBUF_RECORD_ASM_1(0x1, r17, r16, 16, r18, r8, p1, r11)
	{
		memd(r15+#CONTEXT_r0504) = r5:4
		p1 = cmp.eq(r11,#RESCHED_INT)
		r8.h = #hi(BLASTK_inthandlers)
		r18 = sa1
	}
	{
		memd(r15+#CONTEXT_r0302) = r3:2
		r8.l = #lo(BLASTK_inthandlers)
		r17 = m0
	}
	{
		memd(r15+#CONTEXT_r0100) = r1:0
		r8 = addasl(r8,r11,#2)
		r16 = m1
	}
	{
		memd(r15+#CONTEXT_sr_preds) = r29:28
		r29 = add(r15,#THREAD_CONTEXT_TOTALSIZE)
		r8 = memw(r8)
	}
	{
		memd(r15+#CONTEXT_ugpgp) = r23:22
		r3.h = #hi(BLASTK_fastint_contexts)
		r4.h = #hi(BLASTK_fastint_funcptrs)
		r7 = sub(#31,r11)			// what bit to set for ciad
	}
	{
		memd(r15+#CONTEXT_lc0sa0) = r21:20
		r3.l = #lo(BLASTK_fastint_contexts)
		r4.l = #lo(BLASTK_fastint_funcptrs)
		r5 = #0
	}
	{
		memd(r15+#CONTEXT_lc1sa1) = r19:18
		r5 = setbit(r5,r7)			// r5 for ciad
		r6.h = #hi(BLASTK_fastint_gp)
	}
	{
		memd(r15+#CONTEXT_m0m1) = r17:16
		r2 = extractu(r13,#3,#19)
		r6.l = #lo(BLASTK_fastint_gp)
		r0 = r11
	}
	{
		memd(r15+#CONTEXT_ssrelr) = r13:12
		r1 = r15
		r6 = memw(r6)
		callr r8		// do_interrupt or do_int_resched or fast int
	}
	// ~ 28 packets
	.falign
int_context_restore:
	/* r24 - r27 should be the same, from ABI */
	/* SP-THREAD_CONTEXT_TOTALSIZE should be context start. */
	{
		r1:0 = memd(r29+#CONTEXT_ssrelr-THREAD_CONTEXT_TOTALSIZE)
		r3:2 = memd(r29+#CONTEXT_ugpgp-THREAD_CONTEXT_TOTALSIZE)
		r15 = add(r29,#-THREAD_CONTEXT_TOTALSIZE)
		r14 = add(r29,#CONTEXT_sr_preds-THREAD_CONTEXT_TOTALSIZE)
		// possibly, remove above and get a slot for CR XFER?
	}
	{
		r31:30 = memd(r15+#CONTEXT_r3130)
		dcfetch(r14)
		r14 = add(r14,#-32)
		ugp = r3
	}
	{
		r29:28 = memd(r15+#CONTEXT_r2928)
		dcfetch(r14)
		r14 = add(r14,#-32)
		gp = r2
	}
	ssr = r1
	elr = r0
	sgp = r15
	{
		r1:0 = memd(r15+#CONTEXT_sr_preds)
		dcfetch(r14)
		r14 = add(r14,#-32)
		// SLOT FOR CR XFER?
	}
	{
		r19:18 = memd(r15+#CONTEXT_m0m1)
		dcfetch(r14)
		usr = r1
	}
	{
		r23:22 = memd(r15+#CONTEXT_lc1sa1)
		r21:20 = memd(r15+#CONTEXT_lc0sa0)
		p3:0 = r0
	}
	{
		r3:2 = memd(r15+#CONTEXT_r0302)
		r1:0 = memd(r15+#CONTEXT_r0100)
		r19 = r15
		m0 = r19
	}
	{
		r7:6 = memd(r19+#CONTEXT_r0706)
		r5:4 = memd(r19+#CONTEXT_r0504)
		m1 = r18
	}
	{
		r11:10 = memd(r19+#CONTEXT_r1110)
		r9:8 = memd(r19+#CONTEXT_r0908)
		lc0 = r21
	}
	{
		r15:14 = memd(r19+#CONTEXT_r1514)
		r13:12 = memd(r19+#CONTEXT_r1312)
		sa0 = r20
	}
	{
		r19:18 = memd(r19+#CONTEXT_r1918)
		r17:16 = memd(r19+#CONTEXT_r1716)
		r23 = r19
		lc1 = r23
	}
	{
		r23:22 = memd(r23+#CONTEXT_r2322)
		r21:20 = memd(r23+#CONTEXT_r2120)
		sa1 = r22
	}
	rte
	// 16 packets + 28 == 44
FUNC_END BLASTK_handle_int
	

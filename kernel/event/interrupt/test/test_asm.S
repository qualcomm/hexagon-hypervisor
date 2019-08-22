/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/*
 * TH_do_interrupt sets up and calls the interrupt.
 * r0 is the source data to fill into the reg file
 * r1 is the dst location to put into SGP
 * r2 is the interrupt number 
 * 
 * We don't set all registers.  Some registers like gp and SSR
 * might cause some bad things to happen.  The more registers we
 * can check, however, the better.
 */
	.text
	.global TH_do_interrupt
TH_do_interrupt:
	/* Insert interrupt num into cause field */
	r4 = ssr
	r5 = #0xc0
	r4 = insert(r5,#8,#0)
	r4 = insert(r2,#5,#0)
	ssr = r4
#if ARCHV >= 4
	p0 = cmp.geu(r2, #L2_INTERRUPT_START)
	if (!p0) jump 1f
	r5 = #L2_CORE_INTERRUPT
	r4 = insert(r5,#5,#0)
	ssr = r4
	r4 = add(r2,#-32)
	r4 = and(r4,##0x3ff)
	VID = r4
1:
#endif

	/* Put dest into r1 */
	sgp0 = r1

	/* Fill in register values */
	r3:2 = memd(r0+#CONTEXT_ccrssr)
	r5:4 = memd(r0+#CONTEXT_m1m0)
	r7:6 = memd(r0+#CONTEXT_lc1sa1)
	r9:8 = memd(r0+#CONTEXT_lc0sa0)
	r11:10 = memd(r0+#CONTEXT_usrp30)
	r13:12 = memd(r0+#CONTEXT_gpugp)
	//ssr = r3
	//elr = r2
	m1 = r5
	m0 = r4
	lc1 = r7
	sa1 = r6
	lc0 = r9
	sa0 = r8
	usr = r11
	p3:0 = r10
	//gp = r13
	//ugp = r12
	r3:2 = memd(r0+#CONTEXT_r0302)
	r5:4 = memd(r0+#CONTEXT_r0504)
	r7:6 = memd(r0+#CONTEXT_r0706)
	r9:8 = memd(r0+#CONTEXT_r0908)
	r11:10 = memd(r0+#CONTEXT_r1110)
	r13:12 = memd(r0+#CONTEXT_r1312)
	r15:14 = memd(r0+#CONTEXT_r1514)
	r17:16 = memd(r0+#CONTEXT_r1716)
	r19:18 = memd(r0+#CONTEXT_r1918)
	r21:20 = memd(r0+#CONTEXT_r2120)
	r23:22 = memd(r0+#CONTEXT_r2322)
	r25:24 = memd(r0+#CONTEXT_r2524)
	r27:26 = memd(r0+#CONTEXT_r2726)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r31:30 = memd(r0+#CONTEXT_r3130)
	r1:0 = memd(r0+#CONTEXT_r0100)
	jump H2K_handle_int

/*
 * TH_do_preempt sets up and calls the interrupt, as if from the preemption point.
 * r0 is the source data, which is ignored
 * r1 is the dst location to put into r0
 * r2 is the interrupt number 
 * 
 * We don't set all registers.  Some registers like gp and SSR
 * might cause some bad things to happen.  The more registers we
 * can check, however, the better.
 */
	.text
	.global TH_do_preempt
TH_do_preempt:
	/* Insert interrupt num into cause field */
	r4 = ssr
	r5 = #0xc0
	r4 = insert(r5,#8,#0)
	r4 = insert(r2,#5,#0)
	ssr = r4
#if ARCHV >= 4
	p0 = cmp.geu(r2, #L2_INTERRUPT_START)
	if (!p0) jump 1f
	r5 = #L2_CORE_INTERRUPT
	r4 = insert(r5,#5,#0)
	ssr = r4
	r4 = add(r2,#-32)
	r4 = and(r4,##0x3ff)
	VID = r4
1:
#endif

	r0 = #1
	sgp0 = r0
	r0 = r1
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)
	jump H2K_handle_int

/* This is our implementation of the H2_interrupted_fastint_check
 * that may be called.  We set things up and call back to the C
 * function TH_interrupted_fastint_check 
 */
	.global H2K_interrupted_fastint_check
H2K_interrupted_fastint_check:
	r31:30 = memd(r0+#CONTEXT_r3130)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r29 = add(r0,#FASTINT_CONTEXT_SIZE)
	crswap(r0,sgp0)
	jump TH_interrupted_fastint_check

/*
 * H2K_fastint returns immediately 
 */
	.global H2K_fastint
H2K_fastint:
	jumpr r31

/*
 * TH_save_sgp takes the SGP value and saves it into a global memory location 
 * for later restoration 
 */
	.global TH_save_sgp
TH_save_sgp:
	r0.h = #hi(TH_sgp_word)
	r0.l = #lo(TH_sgp_word)
	r1 = sgp0
	memw(r0) = r1
	jumpr r31

/*
 * TH_restore_sgp takes the SGP value from the global 
 * memory location and restores it 
 */
	.global TH_restore_sgp
TH_restore_sgp:
	r0.h = #hi(TH_sgp_word)
	r0.l = #lo(TH_sgp_word)
	r0 = memw(r0)
	sgp0 = r0
	jumpr r31

	.data
	.p2align 2
TH_sgp_word:
	.word 0


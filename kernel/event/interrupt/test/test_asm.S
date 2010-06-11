/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

	.text
	.global TH_do_interrupt
TH_do_interrupt:
	/* Insert interrupt num into cause field */
	r4 = ssr
	r4 = insert(r2,#8,#0)
	ssr = r4

	/* Put dest into r1 */
	sgp = r1

	/* Fill in register values */
	r3:2 = memd(r0+#CONTEXT_ssrelr)
	r5:4 = memd(r0+#CONTEXT_m1m0)
	r7:6 = memd(r0+#CONTEXT_lc1sa1)
	r9:8 = memd(r0+#CONTEXT_lc0sa0)
	r11:10 = memd(r0+#CONTEXT_sr_preds)
	r13:12 = memd(r0+#CONTEXT_ugpgp)
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
	//ugp = r13
	//gp = r12
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

	.global H2K_interrupted_fastint_check
H2K_interrupted_fastint_check:
	r31:30 = memd(r0+#CONTEXT_r3130)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r29 = add(r0,#FASTINT_CONTEXT_SIZE)
	crswap(r0,sgp)
	jump TH_interrupted_fastint_check

	.global H2K_fastint
H2K_fastint:
	jumpr r31

	.global TH_save_sgp
TH_save_sgp:
	r0.h = #hi(TH_sgp_word)
	r0.l = #lo(TH_sgp_word)
	r1 = sgp
	memw(r0) = r1
	jumpr r31

	.global TH_restore_sgp
TH_restore_sgp:
	r0.h = #hi(TH_sgp_word)
	r0.l = #lo(TH_sgp_word)
	r0 = memw(r0)
	sgp = r0
	jumpr r31

	.data
	.p2align 2
TH_sgp_word:
	.word 0


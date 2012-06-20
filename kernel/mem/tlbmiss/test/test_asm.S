/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

	.text
	.global TH_do_tlbmissx
TH_do_tlbmissx:
	/* Insert BADVA,ELR */
#if __QDSP6_ARCH__ >= 4
	badva0 = r3
	badva1 = r3
#else
	badva = r3
#endif
	elr = r4
	/* Insert interrupt num into cause field */
	r4 = ssr
	r4 = insert(r2,#8,#0)
	ssr = r4
	/* Put dest into r1 */
	sgp = r1
	memw(#TH_helper_ref) = r0
	memw(#TH_stack_save) = r29
	/* Fill in register values */
	r3:2 = memd(r0+#CONTEXT_ccrssr)
	r5:4 = memd(r0+#CONTEXT_m1m0)
	r7:6 = memd(r0+#CONTEXT_lc1sa1)
	r9:8 = memd(r0+#CONTEXT_lc0sa0)
	r11:10 = memd(r0+#CONTEXT_sr_preds)
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
	jump H2K_handle_tlbmissx

	.text
	.global TH_do_tlbmissrw
TH_do_tlbmissrw:
	/* Insert BADVA,ELR */
#if __QDSP6_ARCH__ >= 4
	badva0 = r3
	badva1 = r3
#else
	badva = r3
#endif
	elr = r4
	/* Insert interrupt num into cause field */
	r4 = ssr
	r4 = insert(r2,#8,#0)
	ssr = r4
	/* Put dest into r1 */
	sgp = r1
	memw(#TH_helper_ref) = r0
	memw(#TH_stack_save) = r29
	/* Fill in register values */
	r3:2 = memd(r0+#CONTEXT_ccrssr)
	r5:4 = memd(r0+#CONTEXT_m1m0)
	r7:6 = memd(r0+#CONTEXT_lc1sa1)
	r9:8 = memd(r0+#CONTEXT_lc0sa0)
	r11:10 = memd(r0+#CONTEXT_sr_preds)
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
	jump H2K_handle_tlbmissrw

#define CHECK(CTX,REGS) r3:2 = memd(r1+#(CTX)) ; p0 = cmp.eq(r3:2,REGS) ; if (!p0) r0 = #(CTX) ; if (!p0) jump .Ldiff

	.global TH_return_helper
TH_return_helper:
	memd(#TH_helper_tmp) = r1:0
	memd(#TH_helper_tmp2) = r3:2
	r0 = sgp
	r1 = memw(#TH_helper_ref)
	CHECK(CONTEXT_r0504,r5:4)
	CHECK(CONTEXT_r0706,r7:6)
	CHECK(CONTEXT_r0908,r9:8)
	CHECK(CONTEXT_r1110,r11:10)
	CHECK(CONTEXT_r1312,r13:12)
	CHECK(CONTEXT_r1514,r15:14)
	CHECK(CONTEXT_r1716,r17:16)
	CHECK(CONTEXT_r1918,r19:18)
	CHECK(CONTEXT_r2120,r21:20)
	CHECK(CONTEXT_r2322,r23:22)
	CHECK(CONTEXT_r2524,r25:24)
	CHECK(CONTEXT_r2726,r27:26)
	CHECK(CONTEXT_r2928,r29:28)
	CHECK(CONTEXT_r3130,r31:30)
	r5:4 = memd(#TH_helper_tmp2)
	CHECK(CONTEXT_r0302,r5:4)
	r5:4 = memd(#TH_helper_tmp)
	CHECK(CONTEXT_r0100,r5:4)
	r29 = memw(#TH_stack_save)
	r0 = #1
	jump TH_goodreturn

.Ldiff:
	r29 = memw(#TH_stack_save)
	jump TH_goodreturn

	.section .sdata,"aw"
	.p2align 3
TH_helper_tmp:
	.word 0
	.word 0
TH_helper_tmp2:
	.word 0
	.word 0
TH_helper_ref:
	.word 0x80000000
TH_stack_save:
	.word 0x7f000000


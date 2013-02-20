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
 * Also sets up Stack
 */

FUNC_START H2K_context_save .text.core.context .p2align 2
	memd(r0+#CONTEXT_r2928) = r29:28
	//memd(r0+#CONTEXT_r2726) = r27:26
	//memd(r0+#CONTEXT_r2524) = r25:24
	//memd(r0+#CONTEXT_r2322) = r23:22
	//memd(r0+#CONTEXT_r2120) = r21:20
	memd(r0+#CONTEXT_r1918) = r19:18
	memd(r0+#CONTEXT_r1716) = r17:16
	memd(r0+#CONTEXT_r1514) = r15:14
	memd(r0+#CONTEXT_r1312) = r13:12
	memd(r0+#CONTEXT_r1110) = r11:10
	memd(r0+#CONTEXT_r0908) = r9:8
	memd(r0+#CONTEXT_r0706) = r7:6
	memd(r0+#CONTEXT_r0504) = r5:4
	memd(r0+#CONTEXT_r0302) = r3:2
	r7 = r0
	crswap(r0,sgp0)
	memd(r7+#CONTEXT_r0100) = r1:0
	r15 = usr
	r14 = p3:0
	//r23 = gp
	//r22 = ugp
	r19 = lc0
	r18 = sa0
	//r17 = lc1
	//r16 = sa1
	//r15 = m1
	//r14 = m0
	r13 = ssr
	//r12 = elr
	memd(r7+#CONTEXT_sr_preds) = r15:14
	//memd(r7+#CONTEXT_gpugp) = r23:22
	memd(r7+#CONTEXT_lc0sa0) = r19:18
	//memd(r7+#CONTEXT_lc1sa1) = r17:16
	//memd(r7+#CONTEXT_m1m0) = r15:14
	//memd(r7+#CONTEXT_ssrelr) = r13:12
	memw(r7+#CONTEXT_ssr) = r13
#if ARCHV >= 4
	//r27:26 = g1:0
	//r29:28 = g3:2
	//memd(r7+#CONTEXT_gssr_gelr) = r27:26
	//memd(r7+#CONTEXT_gbadva_gosp) = r29:28
	r11 = ccr
	//r21 = cs1
	//r20 = cs0
	memw(r7+#CONTEXT_ccr) = r11
	//memd(r7+#CONTEXT_cs1cs0) = r21:20
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#else
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = extractu(r13,#3,#19)
	r11 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r11)
#endif
	r9:8 = combine(r28,r13)
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)
	callr r31
FUNC_END H2K_context_save

FUNC_START H2K_context_restore_return .text.core.context .p2align 2
	r7 = sgp0
#if ARCHV >= 4
	//r21:20 = memd(r7+#CONTEXT_cs1cs0)
	r11 = memw(r7+#CONTEXT_ccr)
	//cs0 = r20
	//cs1 = r21
	ccr = r11
	//r29:28 = memd(r7+#CONTEXT_gssr_gelr)
	//r27:26 = memd(r7+#CONTEXT_gbadva_gosp)
	//g1:0 = r29:28
	//g3:2 = r27:26
#endif
	r12 = memw(r7+#CONTEXT_ssr)
	//r15:14 = memd(r7+#CONTEXT_m1m0)
	//r17:16 = memd(r7+#CONTEXT_lc1sa1)
	r19:18 = memd(r7+#CONTEXT_lc0sa0)
	//r23:22 = memd(r7+#CONTEXT_gpugp)
	r15:14 = memd(r7+#CONTEXT_sr_preds)
	ssr = r12
	//m0 = r14
	//m1 = r15
	//sa1 = r16
	//lc1 = r17
	sa0 = r18
	lc0 = r19
	//ugp = r22
	//gp = r23
	p3:0 = r14
	usr = r15
	r1:0 = memd(r7+#CONTEXT_r0100)
	crswap(r0,sgp0)
	r3:2 = memd(r0+#CONTEXT_r0302)
	r5:4 = memd(r0+#CONTEXT_r0504)
	r7:6 = memd(r0+#CONTEXT_r0706)
	r9:8 = memd(r0+#CONTEXT_r0908)
	r11:10 = memd(r0+#CONTEXT_r1110)
	r13:12 = memd(r0+#CONTEXT_r1312)
	r15:14 = memd(r0+#CONTEXT_r1514)
	r17:16 = memd(r0+#CONTEXT_r1716)
	r19:18 = memd(r0+#CONTEXT_r1918)
	//r21:20 = memd(r0+#CONTEXT_r2120)
	//r23:22 = memd(r0+#CONTEXT_r2322)
	//r25:24 = memd(r0+#CONTEXT_r2524)
	//r27:26 = memd(r0+#CONTEXT_r2726)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r31:30 = memd(r0+#CONTEXT_r3130)
	crswap(r0,sgp0)
	rte
FUNC_END H2K_context_restore_return


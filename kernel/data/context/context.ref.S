/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <asm_offsets.h>
#include <asm_std.h>
#include <idtype.h>
#include <vmdefs.h>

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
	callr r31  // relies on H2K_context_restore_return being immediately below
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

#ifdef DO_EXT_SWITCH
FUNC_START H2K_ext_context_restore .text.core.context .p2align 2
// r0 = vmblock, r3 = me

	r10 = memw(r3 + #CONTEXT_id)
	r11 = ##H2K_ID_CPUIDX_MASK
	r10 = and(r10, r11)
	r10 = lsr(r10, #H2K_ID_RSVD_BITS) // index
	r12 = ##EXT_CONTEXT_SIZE
	r11 = memw(r0 + #VMBLOCK_ext_contexts) // start of ext contexts
	r11 += mpyu(r12.l, r10.l)  // my ext context

	r13 = ssr
	r13 = setbit(r13, #SSR_XE_BIT)
	ssr = r13

	// predicates
	r14 = add(r11, #(32 * 32))
	v3 = vmem(r14+#0)
	r12 = #1
	r13 = ##0x01010101

	v1 = vsplatw(r13)
	v2 = vand(v3, v1)
	q0 = vcmpb.equ(v2, v1)

	v1 = vaslw(v1, r12) // 0x02020202
	v2 = vand(v3, v1)
	q1 = vcmpb.equ(v2, v1)

	v1 = vaslw(v1, r12) // 0x04040404
	v2 = vand(v3, v1)
	q2 = vcmpb.equ(v2, v1)

	v1 = vaslw(v1, r12) // 0x08080808
	v2 = vand(v3, v1)
	q3 = vcmpb.equ(v2, v1)

	v0 = vmem(r11++#1)
	v1 = vmem(r11++#1)
	v2 = vmem(r11++#1)
	v3 = vmem(r11++#1)
	v4 = vmem(r11++#1)
	v5 = vmem(r11++#1)
	v6 = vmem(r11++#1)
	v7 = vmem(r11++#1)
	v8 = vmem(r11++#1)
	v9 = vmem(r11++#1)
	v10 = vmem(r11++#1)
	v11 = vmem(r11++#1)
	v12 = vmem(r11++#1)
	v13 = vmem(r11++#1)
	v14 = vmem(r11++#1)
	v15 = vmem(r11++#1)
	v16 = vmem(r11++#1)
	v17 = vmem(r11++#1)
	v18 = vmem(r11++#1)
	v19 = vmem(r11++#1)
	v20 = vmem(r11++#1)
	v21 = vmem(r11++#1)
	v22 = vmem(r11++#1)
	v23 = vmem(r11++#1)
	v24 = vmem(r11++#1)
	v25 = vmem(r11++#1)
	v26 = vmem(r11++#1)
	v27 = vmem(r11++#1)
	v28 = vmem(r11++#1)
	v29 = vmem(r11++#1)
	v30 = vmem(r11++#1)
	v31 = vmem(r11++#1)

	// mark ext as live
	r10 = add(r3, ##CONTEXT_atomic_status_word)
1:
	r11 = memw_locked(r10)
	r12 = setbit(r11, #H2K_VMSTATUS_SAVEXT_BIT)
	memw_locked(r10, p0) = r12
	if (!p0) jump 1b

	// enable ext
	r13 = memw(r3 + #CONTEXT_ssr)
	r13 = setbit(r13, #SSR_XE_BIT)
	memw(r3 + #CONTEXT_ssr) = r13

	jump H2K_context_restore_return

FUNC_END H2K_ext_context_restore

#endif

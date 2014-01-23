/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <asm_offsets.h>
#include <asm_std.h>
#include <vmdefs.h>
#include <idtype.h>

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
		memd(r7+#CONTEXT_usrp30) = r19:18
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
		r15:14 = memd(r16+#CONTEXT_usrp30)
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
	rte						// FIXME: simulator bug?  Should be able to schedule w/ above
FUNC_END H2K_context_restore_return

#ifdef DO_EXT_SWITCH
FUNC_START H2K_ext_context_restore .text.core.context .p2align 2
// r0 = vmblock, r3 = me

	{
		r10 = memw(r3 + #CONTEXT_id)
		r11 = ##H2K_ID_CPUIDX_MASK
		r13 = ssr
	}
	{
		r10 = and(r10, r11)
		r12 = ##EXT_CONTEXT_SIZE
		r11 = memw(r0 + #VMBLOCK_ext_contexts) // start of ext contexts
	}
	{
		r10 = lsr(r10, #H2K_ID_VINT_BITS) // index
		r16 = #1
		r13 = setbit(r13, #SSR_XE_BIT)
	}
	{
		r11 += mpyu(r12.l, r10.l)  // my ext context
		r15 = ##0x01010101
		ssr = r13
	}
	{
	 	r14 = add(r11, #(32 * 32))
		v0 = vmem(r11++#1)
	}
	{
		v31 = vmem(r14+#0)
// predicates
	 	v29 = vsplatw(r15)		// 0x01010101
	}
	{
		v30 = vand(v31, v29)
		v1 = vmem(r11++#1)
	}
	{
		q0 = vcmpb.equ(v30, v29)
		v2 = vmem(r11++#1)
	}
	{
		v29 = vaslw(v29, r16) // 0x02020202
		v3 = vmem(r11++#1)
	}
	{
		v30 = vand(v31, v29)
		v4 = vmem(r11++#1)
	}
	{
		q1 = vcmpb.equ(v30, v29)
		v5 = vmem(r11++#1)
	}
	{
		v29 = vaslw(v29, r16) // 0x04040404
		v6 = vmem(r11++#1)
	}
	{
		v30 = vand(v31, v29)
		v7 = vmem(r11++#1)
	}
	{
		q2 = vcmpb.equ(v30, v29)
		v8 = vmem(r11++#1)
	}
	{
		v29 = vaslw(v29, r16) // 0x08080808
		v9 = vmem(r11++#1)
	}
	{
		v30 = vand(v31, v29)
		v10 = vmem(r11++#1)
	}
	{
		q3 = vcmpb.equ(v30, v29)
		v11 = vmem(r11++#1)
	}
	{
// enable ext
	 	r13 = memw(r3 + #CONTEXT_ssr)
		v12 = vmem(r11++#1)
	}
	{
		r13 = setbit(r13, #SSR_XE_BIT)
		v13 = vmem(r11++#1)
	}
	{
		memw(r3 + #CONTEXT_ssr) = r13
		v14 = vmem(r11++#1)
	}
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
	{
		v31 = vmem(r11++#1)
// mark ext as live
		r10 = add(r3, ##CONTEXT_atomic_status_word)
	}
1:
	r11 = memw_locked(r10)
	r12 = setbit(r11, #H2K_VMSTATUS_SAVEXT_BIT)
	memw_locked(r10, p0) = r12
	if (!p0) jump 1b

	jump H2K_context_restore_return

FUNC_END H2K_ext_context_restore

#endif

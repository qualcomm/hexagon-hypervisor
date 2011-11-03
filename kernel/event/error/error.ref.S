/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_handle_error .text.vm.error .p2align 2
	/* TBD: do double exception check as spec'd */
	/* 
	 * XXX: this is rather V3 oriented, we should probably macro 
	 * the guest regs stuff to use guest regs in V4 
	 */
	crswap(r24,sgp)
	memd(r24+#CONTEXT_r1918) = r19:18
	memd(r24+#CONTEXT_r1716) = r17:16
	memd(r24+#CONTEXT_r1514) = r15:14
	memd(r24+#CONTEXT_r1312) = r13:12
	r12 = memw(r24+#CONTEXT_gevb)
	if (r12 == #0) jump:t 1f

	r17 = ssr
	r16 = ELR
	r14 = BADVA
	r13 = p3:0

	memw(r24+#CONTEXT_gssr_gelr) = r16 			// gelr
	memw(r24+#CONTEXT_gbadva_gosp+4) = r14 	// gbadva

#if __QDSP6_ARCH__ >= 4
	g0 = r16
	g3 = r14
#endif

	r14 = memw(r24+#CONTEXT_gbadva_gosp) 		// gosp
	r18 = memw(r24+#CONTEXT_vmstatus)

	p0 = tstbit(r17,#SSR_GUEST_BIT)
	{
		if (!p0) r14 = r29
		if (!p0) r29 = r14
	}

	r15 = zxtb(r17)												// cause
	if (p0) jump 2f 											// !guest == user
	r15 = setbit(r15, #H2K_GSSR_UM_BIT)

2:
	p0 = tstbit(r18,#H2K_VMSTATUS_IE_BIT)	// interrrupts enabled?
	if (!p0) jump 2f
	r15 = setbit(r15, #H2K_GSSR_IE_BIT)

2:
#if __QDSP6_ARCH__ >= 4
	p0 = tstbit(r17,#SSR_SS_BIT)
	if (!p0) jump 2f
	r15 = setbit(r15,#H2K_GSSR_SS_BIT)
#endif

	memw(r24+#CONTEXT_gssr_gelr+4) = r15 		// gssr
	memw(r24+#CONTEXT_gbadva_gosp) = r14 		// gosp

#if __QDSP6_ARCH__ >= 4
	g1 = r15
	g2 = r14
#endif

	r16 = add(r12,#ERROR_GEVB_OFFSET)
	r17 = setbit(r17,#SSR_GUEST_BIT)
#if __QDSP6_ARCH__ >= 4
	r17 = clrbit(r17,#SSR_SS_BIT)
#endif
	memd(r24+#CONTEXT_ssrelr) = r17:16			// FIXME:  necessary?

	// atomic clear vmstatus IE
	r19 = add(r24, #CONTEXT_vmstatus)
2:
	r18 = memw_locked(r19)
	r18 = clrbit(r18, #H2K_VMSTATUS_IE_BIT)
	memw_locked(r19, p0) = r18
	if (!p0) jump 2b

	p3:0 = r13
	elr = r16
	ssr = r17

	r19:18 = memd(r24+#CONTEXT_r1918)
	r17:16 = memd(r24+#CONTEXT_r1716)
	r15:14 = memd(r24+#CONTEXT_r1514)
	r13:12 = memd(r24+#CONTEXT_r1312)
	crswap(r24,sgp)
	rte
1:
	/* Set up KSP and jump to fatal_thread */

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r2 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r2)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif
	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)
	r0 = #-3
	r1 = r24
	r3:2 = #0
	r4 = r28
	crswap(r24,sgp)
	jump H2K_fatal_thread

FUNC_END H2K_handle_error

FUNC_START H2K_handle_nmi .text.vm.error .p2align 2
	crswap(r24,sgp)
	jump 1b
FUNC_END H2K_handle_nmi

FUNC_START H2K_handle_rsvd .text.vm.error .p2align 2
	crswap(r24,sgp)
	jump 1b
FUNC_END H2K_handle_rsvd


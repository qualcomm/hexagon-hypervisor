/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_fastint .text.core.fastint .p2align 5
	// r0 = intno
	// r1 = me... CAN BE NULL
	// r2 = hthread
	// r3 = param
	r15.h = #hi(H2K_fastint_contexts)
	r15.l = #lo(H2K_fastint_contexts)
	r9 = #FASTINT_CONTEXT_SIZE
	r15 += mpyi(r2,r9)

	/* EJP: Not all of the context has been saved/restored 
	 * In particular, many control registers need to be saved
	 * here and restored below.
	 * We will assume, however, that the called function respects the ABI, so 
	 * registers that are fixed (UGP) and callee-save registers are assumed
	 * to be OK not to save
	 */

	r10 = r29
	/* Per-HThread stack may get corrputed by trap in int handler */
	/* So save off important registers in fastint context that won't change */
	/* EJP: Possibly, can avoid save/restore of return since we know continuation */
	r29 = add(r15,#FASTINT_CONTEXT_SIZE)

	allocframe(#16)
	memd(sp+#8) = r19:18
	memd(sp+#0) = r17:16
	r17 = r10

	r18 = r15
	crswap(r18,sgp)
	p0 = cmp.eq(r18,#0)	// swapped from NULL?
	if (p0) jump 1f

	/* Additional Context Save */
	r8 = elr
	memw(r18+#CONTEXT_elr) = r8
	r7 = lc1
	r6 = sa1
	r5 = m1
	r4 = m0
	r13 = gp
	r12 = ugp
	memd(r18+#CONTEXT_lc1sa1) = r7:6
	memd(r18+#CONTEXT_m1m0) = r5:4
	memd(r18+#CONTEXT_gpugp) = r13:12
#if ARCHV >= 4
	r11 = cs1
	r10 = cs0
	memd(r18+#CONTEXT_cs1cs0) = r11:10
#endif

1:
	r19 = #0
	r19 = setbit(r19,r0)
#if ARCHV <= 3
	r19 = brev(r19)
#endif
	r7:6 = memd(H2K_GP+#KG_fastint_gp_ssr)

	// r4 = fastint_functions
	// r5 = mask for ciad
	// r7 = gp
	// r6 = ssr

	r11 = setbit(r11, #SSR_GUEST_BIT)
	r16 = r0		// save intno in r26 for L2 Control
	ssr = r6
	gp = r7
	r6 = #0
	ugp = r6		// clear UGP so we don't mistake the fastint for a real thread
	r0 = add(r0,#-32)

	callr r3

H2K_fastint_return:
	/* L2 Control: if returning TRUE, whack L2 Interrupt Controller */
	H2K_GP.h = #hi(H2K_kg)
	H2K_GP.l = #lo(H2K_kg)
#if H2K_L2_CONTROL
	/* EJP: this should be v4... */
	p0 = cmp.geu(r16, #L2_INTERRUPT_START)
	p1 = cmp.eq(r0,#0)
	r16 = add(r16,#-32)
	if (!p0) jump 1f		// skip if l1 interrupt
	if (p1) jump 4f
	r4 = memw(H2K_GP+#KG_l2_ack_base)
	r4 = tableidxw(r16,#5,#5)
	r5 = and(r16,#0x1f)
	r5 = lsl(#1,r5)
	memw(r4) = r5
4:
	jump 2f
1:
	ciad(r19)
2:
#else
	ciad(r19)
#endif
	//r0 = IPEND
2:
	//p0 = cmp.eq(r0,#0)
	//if (!p0) jump 1f

	p0 = cmp.eq(r18,#0)
	if (p0) jump 1f
	/* Context Restore */
#if ARCHV >= 4
	r11:10 = memd(r18+#CONTEXT_cs1cs0)
	cs0 = r10
	cs1 = r11
#endif
	r5:4 = memd(r18+#CONTEXT_m1m0)
	r7:6 = memd(r18+#CONTEXT_lc1sa1)
	r13:12 = memd(r18+#CONTEXT_gpugp)
	m0 = r4
	m1 = r5
	sa1 = r6
	lc1 = r7
	gp = r13
	ugp = r12
	r8 = memw(r18+#CONTEXT_elr)
	elr = r8
1:

	crswap(r18,sgp)
	/* EJP: Possibly can avoid restore if we know continuation */
	r17:16 = memd(r29+#0)
	r19:18 = memd(r29+#8)
	deallocframe
	r29 = r17
	jumpr r31

FUNC_END H2K_fastint

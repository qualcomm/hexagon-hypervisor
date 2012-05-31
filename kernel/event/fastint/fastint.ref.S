/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_fastint .text.core.fastint .p2align 5
	// r0 = intno
	// r1 = me
	// r2 = hthread
	r3.h = #hi(H2K_fastint_contexts)
	r3.l = #lo(H2K_fastint_contexts)
	r9 = #FASTINT_CONTEXT_SIZE
	r3 += mpyi(r2,r9)

	/* Per-HThread stack may get corrputed by trap in int handler */
	/* So save off important registers in fastint context that won't change */
	/* EJP: Possibly, can avoid save/restore of return since we know continuation */
	memd(r3+#CONTEXT_totalcycles) = r31:30

	r5 = #0
	r5 = setbit(r5,r0)
#if __QDSP6_ARCH__ <= 3
	r5 = brev(r5)
#endif
	r6 = memw(r16+#KG_fastint_gp)

	// r4 = fastint_functions
	// r5 = mask for ciad
	// r6 = gp

	r29 = add(r3,#FASTINT_CONTEXT_SIZE)

	r11.h = #0x01c0
	r11.l = #0x0
	r11 = setbit(r11, #SSR_GUEST_BIT)
	r25:24 = combine(r5,r3)
	r26 = r0		// save intno in r26 for L2 Control
	crswap(r24,sgp)
	ssr = r11
	gp = r6

	r4 = add(r16,#KG_inthandlers)
	r4 = addasl(r4,r0,#3)
	r4 = add(r4, #4)
	r4 = memw(r4)
	callr r4

H2K_fastint_return:
	/* L2 Control: if returning TRUE, whack L2 Interrupt Controller */
#if H2K_L2_CONTROL
	/* EJP: this should be v4... */
	p0 = cmp.gtu(r26,#31)
	p1 = cmp.eq(r0,#0)
	r26 = add(r26,#-32)
	if (!p0) jump 1f		// skip if l1 interrupt
	if (p1) jump 4f
	r4 = memw(r16+#KG_l2_ack_base)
	r4 = tableidxw(r26,#5,#5)
	r5 = and(r26,#0x1f)
	r5 = lsl(#1,r5)
	memw(r4) = r5
4:
	jump 2f
1:
	ciad(r25)
2:
#else
	ciad(r25)
#endif
	//r0 = IPEND
2:
	//p0 = cmp.eq(r0,#0)
	//if (!p0) jump 1f
	crswap(r24,sgp)
	/* EJP: Possibly can avoid restore if we know continuation */
	r31:30 = memd(r24+#CONTEXT_totalcycles)
	jumpr r31

FUNC_END H2K_fastint

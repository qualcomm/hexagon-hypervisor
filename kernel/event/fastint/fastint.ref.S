/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_fastint .text.event.fastint .p2align 5
	// r0 = intno
	// r1 = me
	// r2 = hthread
	r3.h = #hi(H2K_fastint_contexts)
	r3.l = #lo(H2K_fastint_contexts)
	r9 = #FASTINT_CONTEXT_SIZE
	r3 += mpyi(r2,r9)

	/* Per-HThread stack may get corrputed by trap in int handler */
	/* So save off important registers in fastint context that won't change */
	memd(r3+#CONTEXT_oncpu_start) = r31:30
	memd(r3+#CONTEXT_totalcycles) = r25:24

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
	r25:24 = combine(r5,r3)
	crswap(r24,sgp)
	ssr = r11
	gp = r6

	r4 = add(r16,#KG_fastint_funcptrs)
	r4 = addasl(r4,r0,#2)
	r4 = memw(r4)
	callr r4

H2K_fastint_return:
	ciad(r25)
	r0 = IPEND
2:
	p0 = cmp.eq(r0,#0)
	if (!p0) jump 1f
	crswap(r24,sgp)
	r31:30 = memd(r24+#CONTEXT_oncpu_start)
	r25:24 = memd(r24+#CONTEXT_totalcycles)
	jumpr r31

1:
	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)
	r5 = memw(r16+#KG_fastint_mask)
	r4 = add(r5,#KG_fastint_funcptrs)
	r11.h = #0x1c4
	r11.l = #0x0
	r5 = not(r5)
#if __QDSP6_ARCH__ <= 3
	r5 = brev(r5)
#endif
	r13 = imask
	imask = r5
	r14 = ssr
	ssr = r11
	isync
	nop
	ssr = r14
	imask = r13
	r0 = #0
	jump 2b
FUNC_END H2K_fastint

FUNC_START H2K_interrupted_fastint_check .text.event.fastint .p2align 2
	// OK, we checked to see if there was a pending 
	// interrupt, and there sure was!
	// sgp and r0 have been swapped
	// so r0 has our fastint context ptr
	// Note that much of the fastint context has been clobbered
	// r24-27, p1-3, and most lower regs are unchanged.
	// r28-31,r20-23 are saved, but clobbered
	// What do we need to do?
	// restore r29-31 
	// set sgp back to r0
	// fastint_funcptrs in r4
	// Get SSR for new fast interrupt number --> r0
	// Replace SSR w/ precomputed SSR in r14
	// replace IMASK w/ correct IMASK in r13
	// sgp in r24
	// compute new r30 == CIAD mask
	// Last call returned to H2K_fastint_return, so r31 OK as-is
	sgp = r0
	r1 = ssr
	imask = r13
	{
		r29:28 = memd(r0+#CONTEXT_r2928)
		r31:30 = memd(r0+#CONTEXT_r3130)
		r0 = and(r1,#0x1f)
	}
	{
		r4 = addasl(r4,r0,#2)
		r1 = sub(#31,r0)
		r30 = #0
	}
	{
		r4 = memw(r4)
		r30 = setbit(r30,r1)
	}
	ssr = r14
	{
		jumpr r4
	}
FUNC_END H2K_interrupted_fastint_check


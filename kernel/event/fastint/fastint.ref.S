/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

FUNC_START BLASTK_fastint_call .text.event.fastint .p2align 5
	// r0 = intno
	// r1 = me
	// r2 = hthread
	// r3 = fastint_contexts
	// r4 = fastint_functions
	// r5 = mask for ciad
	// r6 = gp
	{
		allocframe(#8)
		r3 += asl(r2,#THREAD_CONTEXT_TOTALSIZE_LOG2)
		r11.h = #0x01c0	// new SSR: pf enabled,um=0,ie=0,ex=0,asid=0
		r4 = addasl(r4,r0,#2)
	}
	{
		memd(r29+#0) = r25:24
		r11.l = #0
		r4 = memw(r4)
		r25:24 = combine(r29,r3)
	}
	crswap(r24,sgp)	// swap fastint_context pointer w/ thread pointer
	ssr = r11
	{
		r29 = add(r3,#THREAD_CONTEXT_TOTALSIZE-128)
		callr r4
		r30 = r5
		gp = r6
	}
BLASTK_fastint_return:
	// EJP May 14
	// It's a real pain to restore all the context, get an interrupt,
	// and then save it all off again if the interrupt is already pending.
	// so, let's check it!
	r0 = IPEND
	ciad(r30)
2:
	{
		p0 = cmp.eq(r0,#0)				// Is IPEND clear?
		if (!p0.new) jump:nt 1f				// no, try and get it without restore/save
		if (p0.new) r31:30 = memd(r25+#8)		// inline deallocframe
		r5.h = #hi(BLASTK_fastint_mask)
	}
	crswap(r24,sgp) // swap back to thread pointer
	// return from fast interrupt, switch back
	{
		r25:24 = memd(r25+#0)
		r29 = add(r25,#16)		// inline deallocframe
		jumpr r31			// yes, return
	}
1:
	{
		// R0 - R15 will be fine through possible interrupt
		r11.h = #0x1c4 // pf enabled,ie=1,ex=0,um=0
		r4.h = #hi(BLASTK_fastint_funcptrs)
		r5.l = #lo(BLASTK_fastint_mask)
	}
	{
		r11.l = #0
		r4.l = #lo(BLASTK_fastint_funcptrs)
		r5 = memw(r5)
	}
	r5 = not(r5)
	r13 = imask
	imask = r5
	r14 = ssr
	ssr = r11
	isync					// SHOULD TAKE INTERRUPT IF PENDING
	nop
	ssr = r14				// interrupts disabled again
	imask = r13				// restore old imask
	{
		r0 = #0				// all for naught, return
		jump 2b
	}
FUNC_END BLASTK_fastint_call

FUNC_START BLASTK_interrupted_fastint_check .text.event.fastint .p2align 2
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
	// Last call returned to BLASTK_fastint_return, so r31 OK as-is
	sgp = r0
	r1 = ssr
	imask = r13
	{
		r29:28 = memd(r0+#CONTEXT_r2928)
		r31:30 = memd(r0+#CONTEXT_r3130)
		r0 = zxtb(r1)
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
FUNC_END BLASTK_interrupted_fastint_check


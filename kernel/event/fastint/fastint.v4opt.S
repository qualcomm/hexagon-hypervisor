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
	// r16 = H2K_kg
	{
		r11 = ##0x01c00000
		r9 = #FASTINT_CONTEXT_SIZE
//		r6 = memw(r16+#KG_fastint_gp)
	}
	// FIXME: not optimized
	r6 = add(r16,#KG_fastint_gps)
	r6 = addasl(r6,r0,#2)
	r6 = memw(r6)
	r11 = setbit(r11, #SSR_GUEST_BIT)
	{
		ssr = r11  // AXOK (if changing bits other than IE, EX, UM)
		r24 = add(##H2K_fastint_contexts,mpyi(r2,r9))
		r4 = add(r16,#KG_fastint_funcptrs)
	}
	{
		/* Per-HThread stack may get corrputed by trap in int handler */
		/* So save off important registers in fastint context that won't change */
		memd(r24+#CONTEXT_reserved_u64) = r31:30
		r4 = memw(r4+r0<<#2)
		r25 = r0
		gp = r6
	}
	{
		memd(r24+#CONTEXT_totalcycles) = r25:24
		r29 = add(r24,#FASTINT_CONTEXT_SIZE)
		callr r4
		crswap(r24,sgp0)
	}
H2K_fastint_return:
	{
		p1 = cmp.gtu(r25,#31)
		if (!p1.new) jump:nt 1f
		p0 = cmp.eq(r0,#0)
		if (p0.new) jump:nt 2f
		r25 = add(r25,#-32)
	}
	{
		r4 = memw(r16+#KG_l2_ack_base)
		r5 = extractu(r25,#5,#5)
		r3 = and(r25,#0x1f)
	}
	{
		r2 = lsl(#1,r3)
		memw(r4+r5<<#2) = r2.new
		jump 2f
	}
1:
	r3 = and(r25,#0x1f)
	r2 = lsl(#1,r3)
	{
		ciad(r2)
	}
2:

//	{
//		r0 = ipend
//		r16 = ##H2K_kg
//		r11.h = #0x1c4
//	}
//	{
		crswap(r24,sgp0)
//		p0 = cmp.eq(r0,#0)
//		if (!p0.new) jump:nt 1f
//	}
	.falign
2:
	{
		r31:30 = memd(r24+#CONTEXT_reserved_u64)
		r25:24 = memd(r24+#CONTEXT_totalcycles)
	}
	{
		jumpr r31
	}
	.falign
//1:
//	{
//		crswap(r24,sgp0)
//	}
//	{
//		r5 = memw(r16+#KG_fastint_mask)
//		r4 = add(r16,#KG_fastint_funcptrs)
//		r13 = imask
//		r11.l = #0x0
//	}
//	{
//		r14 = ssr
//		r5 = not(r5)
//	}
//	imask = r5
//	ssr = r11
//	isync
//	nop
//	ssr = r14
//	imask = r13
//	{
//		jump 2b
//	}
FUNC_END H2K_fastint

#if 0
FUNC_START H2K_interrupted_fastint_check .text.core.fastint .p2align 2
	// OK, we checked to see if there was a pending 
	// interrupt, and there sure was!
	// sgp0 and r0 have been swapped
	// so r0 has our fastint context ptr
	// Note that much of the fastint context has been clobbered
	// r24-27, p1-3, and most lower regs are unchanged.
	// r28-31,r20-23 are saved, but clobbered
	// What do we need to do?
	// restore r29-31 
	// set sgp0 back to r0
	// fastint_funcptrs in r4
	// Get SSR for new fast interrupt number --> r0
	// Replace SSR w/ precomputed SSR in r14
	// replace IMASK w/ correct IMASK in r13
	// sgp0 in r24
	// compute new r30 == CIAD mask
	// Last call returned to H2K_fastint_return, so r31 OK as-is
	{
		r1 = ssr
	}
	{
		r29:28 = memd(r0+#CONTEXT_r2928)
		r31:30 = memd(r0+#CONTEXT_r3130)
		crswap(r0,sgp0)
		r5 = and(r1,#0xff)
	}
	{
		imask = r13  // AXOK
	}
	{
		r4 = memw(r4+r5<<#2)
		r30 = lsl(#1,r5)  // ??? not tested?
		r24 = sgp0
	}
	{
		ssr = r14  // AXOK (if changing bits other than IE, EX, UM)
	}
	{
		r0 = r5
		jumpr r4
	}
FUNC_END H2K_interrupted_fastint_check
#endif

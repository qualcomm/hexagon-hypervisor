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
		r6 = memw(r16+#KG_fastint_gp)
	}
	// FIXME: not optimized
	r11 = setbit(r11, #SSR_GUEST_BIT)
	{
		ssr = r11  // AXOK (if changing bits other than IE, EX, UM)
		r24 = add(##H2K_fastint_contexts,mpyi(r2,r9))
		r4 = add(r16,#KG_inthandlers)
	}
	{
		/* Per-HThread stack may get corrputed by trap in int handler */
		/* So save off important registers in fastint context that won't change */
		memd(r24+#CONTEXT_totalcycles) = r31:30
		r4 = memw(r4+r0<<#3)
		r25 = r0
		gp = r6
	}
	// FIXME: not optimized
	r4 = add(r4, #4)
	{
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
	ciad(r2)
2:
	crswap(r24,sgp0)
	.falign
2:
	r31:30 = memd(r24+#CONTEXT_totalcycles)
	jumpr r31
FUNC_END H2K_fastint


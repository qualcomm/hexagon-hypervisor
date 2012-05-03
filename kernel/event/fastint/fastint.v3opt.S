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
	{
		r3.h = #hi(H2K_fastint_contexts)
		r9 = #FASTINT_CONTEXT_SIZE
		r5 = #0
		r6 = memw(r16+#KG_fastint_gp)
	}
	{
		r3.l = #lo(H2K_fastint_contexts)
		r5 = setbit(r5,r0)
		r11.h = #0x01c0
		gp = r6
	}
	{
		r3 += mpyi(r2,r9)
		r5 = brev(r5)
		r11.l = #0x0
		r4 = add(r16,#KG_fastint_funcptrs)
	}
	// FIXME: not optimized
	r11 = setbit(r11, #SSR_GUEST_BIT)
	{
		/* Per-HThread stack may get corrputed by trap in int handler */
		/* So save off important registers in fastint context that won't change */
		memd(r3+#CONTEXT_totalcycles) = r31:30
		r29 = add(r3,#FASTINT_CONTEXT_SIZE)
		ssr = r11
		r4 = addasl(r4,r0,#2)
	}
	{
		r4 = memw(r4)
		r25:24 = combine(r5,r3)
	}
	{
		callr r4
		crswap(r24,sgp)
	}
H2K_fastint_return:
	ciad(r25)
2:
	crswap(r24,sgp)
	{
		r31:30 = memd(r24+#CONTEXT_totalcycles)
	}
	jumpr r31

FUNC_END H2K_fastint


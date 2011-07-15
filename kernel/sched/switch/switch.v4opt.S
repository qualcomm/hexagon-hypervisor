/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

/*
 * We are switching from thread in r0 to thread in r1
 * We don't care about any registers
 * Handle any tracing and profiling
 * If switching to NULL, go to sleep
 * Otherwise, load continuation into LR and jump to check_sanity_unlock
 */

FUNC_START H2K_switch .text.core.switch .p2align 5
/* EJP: seems like trace is commented out... maybe need to add trace... */
	{
		r25:24 = combine(r1,r0)		// save args for call
		r7 = HTID			// TNUM
		r6 = add(r16,#KG_oncpu_start)
		p0 = cmp.eq(r0,#0)
	}
	{
		r27:26 = s31:30			// PCYCLE HI/LO
		r5:4 = memd(r6+r7<<#3)		// oncpu start
		if (p0) jump .Lfrom_sleep
	}
	{
		r3:2 = sub(r27:26,r5:4)
		r5:4 = memd(r0+#CONTEXT_totalcycles)
	}
	{
		r5:4 = add(r3:2,r5:4)
	}
	{
		memd(r0+#CONTEXT_totalcycles) = r5:4
	}
.Lfrom_sleep:
	{
		p0 = cmp.eq(r1,#0)
		if (p0.new) jump:nt .Lto_sleep
		if (!p0.new) r14 = memub(r1+#CONTEXT_pmu_on)
		if (!p0.new) r31 = memw(r1+#CONTEXT_continuation)
		r15 = PMUCFG
	}
	SGP0 = r1
	{
		memd(r6+r7<<#3) = r27:26
		r4 = memub(r1+#CONTEXT_tid)
		r11:10 = combine(#1,r7)
	}
	{
		r15 = insert(r14,r11:10)
		STID = r1
	}
	{
		PMUCFG = r15
	}
	{
		memb(r1+#CONTEXT_hthread) = r7
		r1:0 = memd(r1+#CONTEXT_r0100)
		jump H2K_check_sanity_unlock
	}
.Lto_sleep:
	{
		r15 = PMUCFG
		r9 = memw(r16+#KG_wait_mask)
		r8 = memw(r16+#KG_priomask)
	}
	{
		r9 = setbit(r9,r7)
		SGP0 = r1
		r3:2 = combine(#0,#-1)
	}
	{
		IMASK = r3
		r8 = setbit(r8,r7)
	}
	{
		memw(r16+#KG_wait_mask) = r9
		memw(r16+#KG_priomask) = r8
		r4 = ssr
		r15 = clrbit(r15,r7)
	}
	{
		STID = r2
		r5 = #4
	}
	{
		r4 = insert(r5,#3,#16)
		PMUCFG = r15
	}
	k0unlock
	{
		ssr = r4
	}
	{
		jump H2K_wait_forever
	}
FUNC_END H2K_switch

FUNC_START H2K_wait_forever .text.core.switch .p2align 2
	wait(r1)
	jump H2K_wait_forever
	nop				// used by Harness to overwrite with redirect
FUNC_END H2K_wait_forever


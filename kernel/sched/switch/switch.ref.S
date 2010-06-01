/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

/*
 * We are switching from thread in r0 to thread in r1
 * We don't care about any registers
 * Handle any tracing and profiling
 * If switching to NULL, go to sleep
 * Otherwise, load continuation into LR and jump to check_sanity_unlock
 */

FUNC_START H2K_switch .text.core.switch .p2align 5
	r25:24 = combine(r1,r0)	// save args for call
	// Get current pcycle count
	r27 = PCYCLEHI
	r26 = PCYCLELO
	r5 = PCYCLEHI
	p0 = cmp.eq(r27,r5)
	if (!p0) jump H2K_switch

#if (__QDSP6_ARCH__) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = HTID
#endif
	r16 = PMUCFG
	r0 = #3
	r3:2 = r27:26
	r10 = r5
	//call H2K_trace
	r1:0 = r25:24

	if (r0 == #0) jump:nt .Lfrom_sleep
	r5:4 = memd(r0+#CONTEXT_oncpu_start)
	r7:6 = sub(r27:26,r5:4)
	r5:4 = memd(r0+#CONTEXT_totalcycles)
	r5:4 = add(r7:6,r5:4)
	memd(r0+#CONTEXT_totalcycles) = r5:4

.Lfrom_sleep:
	if (r1 == #0) jump:nt .Lto_sleep
	memd(r1+#CONTEXT_oncpu_start) = r27:26
	r31 = memw(r1+#CONTEXT_continuation)
	memb(r1+#CONTEXT_hthread) = r10
	r18 = memb(r1+#CONTEXT_pmu_on)
	r11 = #1
	r10 = r5
	r16 = insert(r18,r11:10)
	PMUCFG = r16
	sgp = r1
	r4 = memub(r1+#CONTEXT_tid)
#if (__QDSP6_ARCH__ <= 3)
	tid = r4
#else
	stid = r4
#endif
	r1:0 = memd(r1+#CONTEXT_r0100)
	jump H2K_check_sanity_unlock

.Lto_sleep:
#if (__QDSP6_ARCH__) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = HTID
#endif
	r16 = clrbit(r16,r5)
	PMUCFG = r16
	r9 = memw(r16+#KG_wait_mask)
	r8 = memw(r16+#KG_priomask)
	r9 = setbit(r9,r5)
	r8 = setbit(r8,r5)
	memw(r16+#KG_wait_mask) = r9
	memw(r16+#KG_priomask) = r8
	k0unlock
	// set SGP to NULL
	sgp = r1

	// set IMASK to receive most interrupts
	r0 = imask
	r1.h = #HI(0)
	r1.l = #LO(0)
	r0 = and(r0,r1)
	imask = r0

	// set TID to special value (-1)
	r1 = #-1
#if (__QDSP6_ARCH__ <= 3)
	tid = r1
#else
	stid = r1
#endif

	// set SSR to enable interrupts
	// NOTE: should be the last thing before going to WAIT mode, we may get an interrupt
	// before the actual wait instruction.
	r2 = ssr
	r3 = #4
	r2 = insert(r3,#3,#16)
	ssr = r2
	jump H2K_wait_forever
FUNC_END H2K_switch

FUNC_START H2K_wait_forever .text.core.switch .p2align 2
	wait(r1)
	jump H2K_wait_forever
	nop				// used by Harness to overwrite with redirect
FUNC_END H2K_wait_forever


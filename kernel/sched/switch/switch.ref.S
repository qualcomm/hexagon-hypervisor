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

FUNC_START BLASTK_switch .text.sched.switch .p2align 5
	r25:24 = combine(r1,r0)	// save args for call
	// Get current pcycle count
	r27 = PCYCLEHI
	r26 = PCYCLELO
	r5 = PCYCLEHI
	p0 = cmp.eq(r27,r5)
	if (!p0) jump BLASTK_switch

#if (__QDSP6_ARCH__) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = TNUM
#endif
	r0 = #3
	r3:2 = r27:26
	//call BLASTK_trace
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
	jump BLASTK_check_sanity_unlock

.Lto_sleep:
	k0unlock
	// set SGP to NULL
	sgp = r1

	// set IMASK to receive most interrupts
	r0 = imask
	r1.h = #HI(0)
	r1.l = #LO(0)
	r0 = and(r0,r1)
	imask = r0

	// set SSR to enable interrupts
	r2 = ssr
	r3 = #4
	r2 = insert(r3,#3,#16)
	ssr = r2
	jump BLASTK_wait_forever
FUNC_END BLASTK_switch

FUNC_START BLASTK_wait_forever .text.sched.switch .p2align 2
	wait(r1)
	jump BLASTK_wait_forever
FUNC_END BLASTK_wait_forever


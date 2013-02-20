/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <trace_asm_macro.h>
#include <trace_constants.h>

/*
 * We are switching from thread in r0 to thread in r1
 * We don't care about any registers
 * Handle any tracing and profiling
 * If switching to NULL, go to sleep
 * Otherwise, load continuation into LR and jump to check_sanity_unlock
 */

FUNC_START H2K_switch .text.core.switch .p2align 5
	r17:16 = combine(r1,r0)	// save args for call
	// Get current pcycle count
	r19 = PCYCLEHI
	r18 = PCYCLELO
	r5 = PCYCLEHI
	p0 = cmp.eq(r19,r5)
	if (!p0) jump H2K_switch

#if (ARCHV) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = HTID
#endif
	r7:6 = combine(r5,r18)
	r15 = PMUCFG
	r0 = #3
	r3:2 = r19:18
	r10 = r5
	//call H2K_trace
	r1:0 = r17:16
	r8 = #KG_oncpu_start
	r8 = addasl(r8,r5,#3)
	r8 = add(H2K_GP,r8)

	r9 = #KG_oncpu_wait
	r9 = addasl(r9,r5,#3)
	r9 = add(H2K_GP,r9)

	r11 = #KG_waitcycles
	r11 = addasl(r11,r5,#3)
	r11 = add(H2K_GP,r11)

	p1 = cmp.eq(r0,#0)
	if (p1) jump .Lfrom_sleep

	r5:4 = memd(r8)
	r7:6 = sub(r19:18,r5:4)
	r5:4 = memd(r0+#CONTEXT_totalcycles)
	r5:4 = add(r7:6,r5:4)
	memd(r0+#CONTEXT_totalcycles) = r5:4

	memd(r0+#CONTEXT_r2120) = r21:20
	memd(r0+#CONTEXT_r2322) = r23:22
	memd(r0+#CONTEXT_r2524) = r25:24
	memd(r0+#CONTEXT_r2726) = r27:26

	r14 = elr
	r25 = m1
	r24 = m0
	r23 = gp
	r22 = ugp
	r21 = lc1
	r20 = sa1

	memw(r0+#CONTEXT_elr) = r14
	memd(r0+#CONTEXT_gpugp) = r23:22
	memd(r0+#CONTEXT_lc1sa1) = r21:20
	memd(r0+#CONTEXT_m1m0) = r25:24

#if ARCHV >= 4
	r27:26 = g3:2
	r25:24 = g1:0
	r21 = cs1
	r20 = cs0
	r14 = ccr
	memd(r0+#CONTEXT_gbadva_gosp) = r27:26
	memd(r0+#CONTEXT_gssr_gelr) = r25:24
	memd(r0+#CONTEXT_cs1cs0) = r21:20
	memw(r0+#CONTEXT_ccr) = r14
#endif

.Lfrom_sleep:
	if (r1 == #0) jump:nt .Lto_sleep
	r27:26 = combine(r10,r18)
	r20 = #H2K_TRACE_SWITCH
	r14 = memw(r1+#CONTEXT_id)
	r14 = insert(r20,#8,#0)
	ASM_INLINE_TRACE(r14,r27:26,r27,r26,r25:24,r25,r24,p0,r21,r20,r31,,)
	memd(r8) = r19:18

#if ARCHV >= 4
	r14 = memw(r1+#CONTEXT_ccr)
	r21:20 = memd(r1+#CONTEXT_cs1cs0)
	r25:24 = memd(r1+#CONTEXT_gssr_gelr)
	r27:26 = memd(r1+#CONTEXT_gbadva_gosp)
	ccr = r14
	cs0 = r20
	cs1 = r21
	g1:0 = r25:24
	g3:2 = r27:26
#endif

	r14 = memw(r1+#CONTEXT_elr)
	r23:22 = memd(r1+#CONTEXT_gpugp)
	r21:20 = memd(r1+#CONTEXT_lc1sa1)
	r25:24 = memd(r1+#CONTEXT_m1m0)

	elr = r14
	m1 = r25
	m0 = r24
	gp = r23
	ugp = r22
	lc1 = r21
	sa1 = r20

	r21:20 = memd(r1+#CONTEXT_r2120)
	r23:22 = memd(r1+#CONTEXT_r2322)
	r25:24 = memd(r1+#CONTEXT_r2524)
	r27:26 = memd(r1+#CONTEXT_r2726)

	r5:4 = memd(r9)
	r7:6 = sub(r19:18,r5:4)
	r5:4 = memd(r11)
	r5:4 = add(r7:6,r5:4)
	if (p1) memd(r11) = r5:4

	memb(r1+#CONTEXT_hthread) = r10
	r14 = memb(r1+#CONTEXT_pmu_on)
	r11 = #1
	r10 = r5
	r15 = insert(r14,r11:10)
	PMUCFG = r15
	sgp0 = r1
	r4 = memw(r1+#CONTEXT_status_prio_hthread_tid)
#if (ARCHV <= 3)
	tid = r4
#else
	stid = r4
#endif
	r17 = r1
	r0 = r17
	call H2K_vm_do_work
	r1:0 = memd(r17+#CONTEXT_r0100)
	r31 = memw(r17+#CONTEXT_continuation)
	jump H2K_check_sanity_unlock

.Lto_sleep:
	memd(r9) = r19:18
#if (ARCHV) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = HTID
#endif
	r27:26 = combine(r5,r18)
	r14 = #H2K_TRACE_SWITCH
	ASM_INLINE_TRACE(r14,r27:26,r27,r26,r25:24,r25,r24,p0,r21,r20,r31,,)

	r15 = clrbit(r15,r5)
	PMUCFG = r15
	r9 = memw(H2K_GP+#KG_wait_mask)
	r8 = memw(H2K_GP+#KG_priomask)
	r9 = setbit(r9,r5)
	r8 = setbit(r8,r5)
	memw(H2K_GP+#KG_wait_mask) = r9
	memw(H2K_GP+#KG_priomask) = r8
	k0unlock
	// set SGP to NULL
	sgp0 = r1

	// set IMASK to receive most interrupts
	r0 = imask
	r1.h = #HI(0)
	r1.l = #LO(0)
	r0 = and(r0,r1)
	imask = r0

	// set TID to special value (-1)
	r1 = #-1
#if (ARCHV <= 3)
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


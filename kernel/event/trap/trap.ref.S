/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

/*
 * optimization note:
 * 
 * The first two cache lines of the context should be prefetched.
 * The next several cache lines of the context should be DCZERO'd.
 */

FUNC_START H2K_handle_trap0 .text.event.trap0 .p2align 2
	crswap(r7,sgp)
	// Save trap state
	r13 = ugp
	r12 = gp
	memd(r7+#CONTEXT_r3130) = r31:30
	memd(r7+#CONTEXT_r2928) = r29:28
	memd(r7+#CONTEXT_r2726) = r27:26
	memd(r7+#CONTEXT_r2524) = r25:24
	memd(r7+#CONTEXT_r2322) = r23:22
	memd(r7+#CONTEXT_r2120) = r21:20
	memd(r7+#CONTEXT_r1918) = r19:18
	memd(r7+#CONTEXT_r1716) = r17:16
	r16 = elr
	r17 = ssr
	memd(r7+#CONTEXT_ssrelr) = r17:16
	r18 = p3:0
#if __QDSP6_ARCH__ >= 4
	r19 = ccr
#endif
	r21 = ugp
	r20 = gp
	memd(r7+#CONTEXT_ugpgp) = r21:20
	memw(r7+#CONTEXT_ccr) = r19

	// Can we take this trap?
	r23 = and(r17,#0xff)
	p0 = cmp.gt(r23,#0x1f)
	if (p0) jump 1f					// no, too big.
	r22 = memw(r7+#CONTEXT_trapmask)
	p0 = tstbit(r22,r23)
	if (!p0) jump 1f				// no, disabled.

	/* OK to clobber caller saves */
	sgp = r7

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r29 += mpyi(r28,#KERNEL_STACK_SIZE)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif

	r22.h = #hi(H2K_traptab)
	r22.l = #lo(H2K_traptab)
	r22 = addasl(r22,r23,#3)

	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)

	// set up continuation in case we block
	call .Ljump_to_trap

H2K_trap_nogood:
	r16 = sgp

	r19:18 = memd(r16+#CONTEXT_ugpgp)
	ugp = r19
	gp  = r18
	r19:18 = memd(r16+#CONTEXT_ssrelr)
	ssr = r19
	elr = r18
	r17 = memw(r16+#CONTEXT_ccr)
#if __QDSP6_ARCH__ >= 4
	ccr = r17
#endif
	r31:30 = memd(r16+#CONTEXT_r3130)
	r29:28 = memd(r16+#CONTEXT_r2928)
	r27:26 = memd(r16+#CONTEXT_r2726)
	r25:24 = memd(r16+#CONTEXT_r2524)
	r23:22 = memd(r16+#CONTEXT_r2322)
	r21:20 = memd(r16+#CONTEXT_r2120)
	r19:18 = memd(r16+#CONTEXT_r1918)
	r17:16 = memd(r16+#CONTEXT_r1716)

9:
	rte

1:
	/* if gevb, do guest event */
	r23 = memw(r7+#CONTEXT_gevb)
	if (r23 != #0) jump:t .Lguest_trap
	// call H2K_fatal_thread or H2K_fatal_kernel
	r0 = #-1
	r1 = r7
	r3 = pcyclehi
	r2 = pcyclelo
	r4 = ssr
	p0 = tstbit(r4,#18)			// interrupts enabled?
#if __QDSP6_ARCH__ <= 3
	r4 = extractu(r4,#3,#19)
#else
	r4 = htid
#endif
	if (!p0) jump 1f
	jump H2K_fatal_thread		// no, regular thread
1:
	jump H2K_fatal_kernel		// yes, fast interrupt

	// jump to r10 after saving continuation
.Ljump_to_trap:
	memw(r7+#CONTEXT_continuation) = r31
	jumpr r22

	/* EJP: all context carefully saved */
	/* We have a disabled or other trap failure, return to guest */
	/* GEVB is in r23 */
	/* Preds are in R18 */
	/* r17:16 == ssr:elr */
.Lguest_trap:
	r27:26 = combine(r16,r16)
	memd(r7+#CONTEXT_gelr_gbadva) = r27:26
	r25:24 = memd(r7+#CONTEXT_gssr_gosp)
	p0 = tstbit(r17,#SSR_GUEST_BIT)
	{
		if (!p0) r24 = r29
		if (!p0) r29 = r24
	}
	memw(r7+#CONTEXT_r2928+4) = r29
	/* TBD: set up GSSR in r25 */
	memd(r7+#CONTEXT_gssr_gosp) = r25:24
	r16 = add(r23,#TRAP0_GEVB_OFFSET)
	r17 = setbit(r17,#SSR_GUEST_BIT)
	memd(r7+#CONTEXT_ssrelr) = r17:16
	p3:0 = r18
	crswap(r7,sgp)
	jump H2K_trap_nogood

FUNC_END H2K_handle_trap0

FUNC_START H2K_traptab .text.event.trap0 .p2align 8
	{
		jump H2K_trap_angel // 0
		nop
	}
	{
		r0 = r7
		jump H2K_thread_id // 1
	}
	{
		r2 = r7
		jump H2K_futex_wait // 2
	}
	{
		r2 = r7
		jump H2K_futex_resume // 3
	}
	{
		r5 = r7
		jump H2K_thread_create // 4
	}
	{
		r0 = r7
		jump H2K_thread_stop // 5
	}
	{
		r0 = r7
		jump H2K_cputime_get // 6
	}
	{
		r0 = r7
		jump H2K_thread_id // 7 // jumpr r31?
	}
	{
		r2 = r7
		jump H2K_register_fastint // 8
	}
	{
		r2 = r7
		jump H2K_prio_set // 9
	}
	{
		r1 = r7
		jump H2K_prio_get // 10
	}
	{
		//r2 = r7
		//jump H2K_change_asid_mappings // 11
		r0 = r7
		jump H2K_thread_id
	}
	{
		r0 = r7
		jump H2K_sched_yield // 12
	}
	{
		//r1 = r7
		//jump H2K_mask_prios_above // 13
		r0 = r7
		jump H2K_thread_id
	}
	{
		r0 = r7
		jump H2K_thread_id // 14
	}
	{
		r0 = r7
		jump H2K_thread_id // 15
	}
	{
		r0 = r7
		jump H2K_pcycles_get // 16
	}
	{
		//r1 = r7
		//jump H2K_set_prefetch // 17
		r0 = r7
		jump H2K_thread_id
	}
	{
		r1 = r7
		jump H2K_tid_set // 18
	}
	{
		r0 = r7
		jump H2K_tid_get // 19
	}
	{
		r1 = r7
		jump H2K_futex_lock_pi // 20
	}
	{
		r1 = r7
		jump H2K_futex_unlock_pi // 21
	}
	{
		r0 = r7
		jump H2K_thread_id // 22
	}
	{
		r0 = r7
		jump H2K_thread_id // 23
	}
	{
		r0 = r7
		jump H2K_thread_id // 24
	}
	{
		r0 = r7
		jump H2K_thread_id // 25
	}
	{
		r0 = r7
		jump H2K_thread_id // 26
	}
	{
		r0 = r7
		jump H2K_thread_id // 27
	}
	{
		r0 = r7
		jump H2K_thread_id // 28
	}
	{
		r0 = r7
		jump H2K_thread_id // 29
	}
	{
		r4 = r7
		jump H2K_trap_config // 30
	}
	{
		r0 = r7
		jump H2K_thread_id // 31
	}
FUNC_END H2K_traptab


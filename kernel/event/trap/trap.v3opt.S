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

FUNC_START H2K_handle_trap0 .text.core.trap0 .p2align 2
	{
		crswap(r7,sgp)
	}
	{
		dcfetch(r7)  // dcfetch first line
	}
	{
		dcfetch(r7+#32)  // dcfetch second line
		r7 = add(r7,#64)  // compute dczeroa pointer
	}
	{
		dczeroa(r7)  // dczero third line
		r7 = add(r7,#32)  // increment dczeroa pointer
	}
	{
		dczeroa(r7)  // dczero fourth line
		r7 = add(r7,#32)  // increment dczeroa pointer
	}
	{
		dczeroa(r7)  // dczero fifth line
		r7 = add(r7,#-128)  // restore r7 to context pointer
	}
		// Save trap state
	{
		memd(r7+#CONTEXT_r3130) = r31:30
		r31 = ssr
	}
	{
		r30 = elr  // keep elr in r30 throughout function
		memd(r7+#CONTEXT_r2726) = r27:26
		r27 = and(r31,#0xff)  // extract cause code
		r26.h = #hi(H2K_stacks)
	}
	{
		memd(r7+#CONTEXT_r2524) = r25:24
		r25 = p3:0
		r26.l = #lo(H2K_stacks)
	}
	{
		memd(r7+#CONTEXT_r2322) = r23:22
		r23 = ugp
	}
	{
		memd(r7+#CONTEXT_r2120) = r21:20
		r22 = gp
		r24 = memw(r7+#CONTEXT_trapmask)
	}
	{
		memd(r7+#CONTEXT_r1716) = r17:16
		// Can we take this trap?
		p0 = cmp.gt(r27,#0x1f)  // trap too big?
		p1 = tstbit(r24,r27)  // trap enabled?
		r16 = r31  // put ssr in r16
	}
	{
		memd(r7+#CONTEXT_r1918) = r19:18
		r17.h = #hi(H2K_traptab)
		p1 = not(p1)  // trap disabled?
		r18 = extractu(r31,#3,#19)  // extract htid
	}
	{
		memd(r7+#CONTEXT_ssrelr) = r31:30
		r19 = memw(r7+#CONTEXT_gevb)  // keep gevb in r19 throughout function
		r17.l = #lo(H2K_traptab)
		r31 = r30  // r31:30 = elr:elr
	}
	{
		memd(r7+#CONTEXT_ugpgp) = r23:22
		r26 += mpyi(r18,#KERNEL_STACK_SIZE)
		p0 = or(p0,p1)
		p1 = cmp.eq(r19,#0)
	}
	{
		memd(r7+#CONTEXT_r2928) = r29:28
		r17 += asl(r27,#3)
		if (p0) jump .Lreturn_to_guest  // no, too big or disabled.
	}

		/* OK to clobber caller saves */
	{
		r29 = r26  // ??? not tested?
		r16.h = #hi(H2K_kg)  // ??? not tested?
		call .Ljump_to_trap
	}
	.falign
H2K_trap_nogood:
	{
		r16 = sgp
	}
	{
		r19:18 = memd(r16+#CONTEXT_ugpgp)
		r31:30 = memd(r16+#CONTEXT_r3130)
	}
	{
		ugp = r19
		r29:28 = memd(r16+#CONTEXT_r2928)
		r27:26 = memd(r16+#CONTEXT_r2726)
	}
	{
		gp  = r18
		r25:24 = memd(r16+#CONTEXT_r2524)
		r19:18 = memd(r16+#CONTEXT_ssrelr)
	}
	{
		ssr = r19  // AXOK
	}
	{
		r23:22 = memd(r16+#CONTEXT_r2322)
		r21:20 = memd(r16+#CONTEXT_r2120)
	}
	{
		elr = r18  // AXOK
	}
	{
		r19:18 = memd(r16+#CONTEXT_r1918)
		r17:16 = memd(r16+#CONTEXT_r1716)
	}
	{
		rte  // ??? AXOK?
	}

	.falign
.Ljump_to_trap:
	{
		r16.l = #lo(H2K_kg)
		memw(r7+#CONTEXT_continuation) = r31  // set up continuation in case we block  ??? not tested?
		crswap(r7,sgp)  // ??? sgp = r7 is AXOK?
		jumpr r17
	}

	.falign
.Lreturn_to_guest:
	/* EJP: all context carefully saved */
	/* We have a disabled or other trap failure, return to guest */
	/* GEVB is in r19 */
	/* Preds are in r25 */
	/* r31:30 == elr:elr */
	/* r16 == ssr */
	/* p1 == (gevb == 0) */
	{
		if (p1) jump .Lfatal
		// if gevb, do guest event
		if (!p1) memd(r7+#CONTEXT_gelr_gbadva) = r31:30  // elr:elr
		r23:22 = memd(r7+#CONTEXT_gssr_gosp)
		p0 = tstbit(r16,#SSR_GUEST_BIT)
	}
	{
		if (!p0) r22 = r29
		if (!p0) r29 = r22
		/* TBD: set up GSSR in r23 */
		r23 = zxtb(r16)
	}
	{
		memd(r7+#CONTEXT_gssr_gosp) = r23:22  // ??? not tested?
	}
	{
		memw(r7+#CONTEXT_r2928+4) = r29
		r20 = add(r19,#TRAP0_GEVB_OFFSET)
		r21 = setbit(r16,#SSR_GUEST_BIT)
		r31:30 = memd(r7+#CONTEXT_r3130)
	}
	{
		memd(r7+#CONTEXT_ssrelr) = r21:20
		r29:28 = memd(r7+#CONTEXT_r2928)
		p3:0 = r25
	}
	{
		ssr = r21  // AXOK (if changing bits other than IE, EX, UM)
	}
	{
		elr = r20  // AXOK
	}
	{
		r27:26 = memd(r7+#CONTEXT_r2726)
		r25:24 = memd(r7+#CONTEXT_r2524)
	}
	{
		r23:22 = memd(r7+#CONTEXT_r2322)
		r21:20 = memd(r7+#CONTEXT_r2120)
	}
	{
		r19:18 = memd(r7+#CONTEXT_r1918)
		r17:16 = memd(r7+#CONTEXT_r1716)
		crswap(r7,sgp)
	}
	{
		rte  // ??? AXOK?
	}

		// call H2K_fatal_thread or H2K_fatal_kernel
	.falign
.Lfatal:
	{
		r1 = r7
		r4 = ssr
	}
	{
		r0 = pcyclehi
		p0 = tstbit(r4,#18)  // interrupts enabled?
	}
	{
		r2 = pcyclelo
		r4 = extractu(r4,#3,#19)
	}
	{
		r3 = pcyclehi  // read pcyclehi again to see if it incremented
	}
	{
		p1 = cmp.eq(r3,r0)  // did pcyclehi increment?
		if (!p1.new) r2 = #0  // if so, then use 0 for pcyclelo
		r0 = #-1
		if (!p0) jump 1f
	}
	{
		jump H2K_fatal_thread  // interrupts enabled, regular thread
	}
1:
	{
		jump H2K_fatal_kernel  // interrupts disabled, fast interrupt
	}

FUNC_END H2K_handle_trap0

FUNC_START H2K_traptab .text.core.trap0 .p2align 8
	{
		jump H2K_trap_angel // 0
		nop
	}
	{
		r0 = sgp
		jump H2K_thread_id // 1
	}
	{
		r2 = sgp
		jump H2K_futex_wait // 2
	}
	{
		r2 = sgp
		jump H2K_futex_resume // 3
	}
	{
		r5 = sgp
		jump H2K_thread_create // 4
	}
	{
		r0 = sgp
		jump H2K_thread_stop // 5
	}
	{
		r0 = sgp
		jump H2K_cputime_get // 6
	}
	{
		r0 = sgp
		jump H2K_thread_id // 7 // jumpr r31?
	}
	{
		r2 = sgp
		jump H2K_register_fastint // 8
	}
	{
		r2 = sgp
		jump H2K_prio_set // 9
	}
	{
		r1 = sgp
		jump H2K_prio_get // 10
	}
	{
		//r2 = r7
		//jump H2K_change_asid_mappings // 11
		r0 = sgp
		jump H2K_thread_id
	}
	{
		r0 = sgp
		jump H2K_sched_yield // 12
	}
	{
		//r1 = r7
		//jump H2K_mask_prios_above // 13
		r0 = sgp
		jump H2K_thread_id
	}
	{
		r0 = sgp
		jump H2K_thread_id // 14
	}
	{
		r0 = sgp
		jump H2K_thread_id // 15
	}
	{
		r0 = sgp
		jump H2K_pcycles_get // 16
	}
	{
		//r1 = r7
		//jump H2K_set_prefetch // 17
		r0 = sgp
		jump H2K_thread_id
	}
	{
		r1 = sgp
		jump H2K_tid_set // 18
	}
	{
		r0 = sgp
		jump H2K_tid_get // 19
	}
	{
		r1 = sgp
		jump H2K_futex_lock_pi // 20
	}
	{
		r1 = sgp
		jump H2K_futex_unlock_pi // 21
	}
	{
		r0 = sgp
		jump H2K_thread_id // 22
	}
	{
		r0 = sgp
		jump H2K_thread_id // 23
	}
	{
		r0 = sgp
		jump H2K_thread_id // 24
	}
	{
		r0 = sgp
		jump H2K_thread_id // 25
	}
	{
		r0 = sgp
		jump H2K_thread_id // 26
	}
	{
		r0 = sgp
		jump H2K_thread_id // 27
	}
	{
		r5 = sgp
		jump H2K_vmboot // 28
	}
	{
		r4 = sgp
		jump H2K_trap_pmuconfig // 29
	}
	{
		r5 = sgp
		jump H2K_trap_config // 30
	}
	{
		r4 = sgp
		jump H2K_trap_hwconfig // 31
	}
FUNC_END H2K_traptab

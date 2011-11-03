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
		crswap(r7,sgp0)
	}
	{
		dcfetch(r7)  // dcfetch first line
	}
	{
		dcfetch(r7+#32)  // dcfetch second line
		r7 = add(r7,#96)  // compute dczeroa pointer
	}
	{
		dczeroa(r7)  // dczero fourth line
		r7 = add(r7,#32)  // increment dczeroa pointer
	}
	{
		dczeroa(r7)  // dczero fifth line
		r7 = add(r7,#-64)  // decrement dczeroa pointer
	}
	{
		dczeroa(r7)  // dczero third line
		r7 = add(r7,#-64)  // restore r7 to context pointer
	}
		// Save trap state
	{
		memd(r7+#CONTEXT_r2322) = r23:22
		memd(r7+#CONTEXT_r2524) = r25:24
		r23:22 = s7:6  // ccr:ssr, keep ssr in r22
		r25 = #KERNEL_STACK_SIZE
	}
	{
		memd(r7+#CONTEXT_r2726) = r27:26
		memd(r7+#CONTEXT_r2120) = r21:20
		r27:26 = c11:10  // gp:ugp
		r21 = r22
	}
	{
		memd(r7+#CONTEXT_r1918) = r19:18
		memd(r7+#CONTEXT_r1716) = r17:16
		r20 = elr  // keep elr in r20
		r17 = and(r22,#0xff)  // extract cause code
	}
	{
		memd(r7+#CONTEXT_ssrelr) = r21:20
		memd(r7+#CONTEXT_gpugp) = r27:26
		r26 = htid
		r21 = r20  // r21:20 = elr:elr
	}
	{
		r19 = memw(r7+#CONTEXT_gevb)  // keep gevb in r19
		r18 = memw(r7+#CONTEXT_trapmask)
		r24 = ##H2K_traptab
		r27 = p3:0  // keep p3:0 in r27
	}
	{
		memd(r7+#CONTEXT_r3130) = r31:30
		memd(r7+#CONTEXT_r2928) = r29:28
		// Can we take this trap?
		p0 = !cmp.gt(r17,#0x1f)  // trap not too big?
		p0 = tstbit(r18,r17)  // trap enabled?
	}
	{
		memw(r7+#CONTEXT_ccr) = r23
		p1 = cmp.eq(r19,#0)
		if (!p0) jump .Lreturn_to_guest  // no, too big or disabled.
		r24 += asl(r17,#3)
	}
		/* OK to clobber caller saves */
	{
		r16.h = #hi(H2K_kg)  // ??? not tested?
		r29 = add(##H2K_stacks,mpyi(r26,r25))  // ??? not tested?
		call .Ljump_to_trap
	}
	.falign
H2K_trap_nogood:
	r17 = sgp
	{
		memd(r17+#CONTEXT_r0100) = r1:0
		call H2K_preempt
	}
	{
		r19 = memw(r17+#CONTEXT_ccr)
	}
	{
		ccr = r19  // AXOK
	}
	{
		r19:18 = memd(r17+#CONTEXT_gpugp)
		r31:30 = memd(r17+#CONTEXT_r3130)
	}
	{
		c11:10 = r19:18 // gp:ugp
		r29:28 = memd(r17+#CONTEXT_r2928)
		r27:26 = memd(r17+#CONTEXT_r2726)
	}
	{
		r25:24 = memd(r17+#CONTEXT_r2524)
		r19:18 = memd(r17+#CONTEXT_ssrelr)
	}
	{
		ssr = r19  // AXOK (if changing bits other than IE, EX, UM)
	}
	{
		r23:22 = memd(r17+#CONTEXT_r2322)
		r21:20 = memd(r17+#CONTEXT_r2120)
	}
	{
		elr = r18  // AXOK
	}
	{
		r19:18 = memd(r17+#CONTEXT_r1918)
		r17:16 = memd(r17+#CONTEXT_r1716)
	}
	{
		rte  // ??? AXOK?
	}

	.falign
.Ljump_to_trap:
	{
		r16.l = #lo(H2K_kg)
		memw(r7+#CONTEXT_continuation) = r31  // set up continuation in case we block  ??? not tested?
		crswap(r7,sgp0)  // ??? sgp0 = r7 is AXOK?
		jumpr r24
	}

	.falign
.Lreturn_to_guest:
	/* EJP: all context carefully saved */
	/* We have a disabled or other trap failure, return to guest */
	/* GEVB is in r19 */
	/* Preds are in r27 */
	/* r21:20 == elr:elr */
	/* r22 == ssr */
	/* p1 == (gevb == 0) */
//	{
	if (p1) jump .Lfatal
	// if gevb, do guest event
	if (!p1) memw(r7+#CONTEXT_gssr_gelr) = r21  // elr:elr
	if (!p1) memw(r7+#CONTEXT_gbadva_gosp+4) = r20  // elr:elr

	g0 = r20
  g3 = r20		

//	}
	{
		r25 = memw(r7+#CONTEXT_gssr_gelr+4) 	// gssr
		r24 = memw(r7+#CONTEXT_gbadva_gosp) 	// gosp
	}
	{
		r26 = memw(r7+#CONTEXT_vmstatus)
		p0 = tstbit(r22,#SSR_GUEST_BIT)
	}
	{
		if (!p0) r24 = r29
		if (!p0) r29 = r24
		r25 = zxtb(r24)											  // cause
		if (p0) jump 1f 											// !guest == user
	}
	r25 = setbit(r25, #H2K_GSSR_UM_BIT)

1:
	{
		p0 = tstbit(r26, #H2K_VMSTATUS_IE_BIT)					// interrrupts enabled?
		if (!p0.new) jump:nt 1f
	}
	r25 = setbit(r25, #H2K_GSSR_IE_BIT)

1:
	{
		p0 = tstbit(r22,#SSR_SS_BIT)
		if (!p0.new) jump:nt 1f
	}
	r25 = setbit(r25,#H2K_GSSR_SS_BIT)
//	{
	memw(r7+#CONTEXT_gssr_gelr+4) = r25  	// gssr
	memw(r7+#CONTEXT_gbadva_gosp) = r24 	// gosp

	g1 = r25
  g2 = r24

//	}
	{
		memw(r7+#CONTEXT_r2928+4) = r29
		r20 = add(r19,#TRAP0_GEVB_OFFSET)
		r21 = setbit(r22,#SSR_GUEST_BIT)
	}

	  // atomic clear vmstatus IE
	{
		r19 = add(r7, #CONTEXT_vmstatus)
		r21 = clrbit(r22,#SSR_SS_BIT)
	}
1:
	r18 = memw_locked(r19)
	r18 = clrbit(r18, #H2K_VMSTATUS_IE_BIT)
	memw_locked(r19, p0) = r18
	if (!p0) jump 1b

	{
		memd(r7+#CONTEXT_ssrelr) = r21:20
		r27:26 = memd(r7+#CONTEXT_r2726)
		p3:0 = r27
	}
	{
		ssr = r21  // AXOK (if changing bits other than IE, EX, UM)
	}
	{
	  elr = r20  // AXOK
	}
	{
//		r31:30 = memd(r7+#CONTEXT_r3130)  FIXME: r31:30 not modified
		r25:24 = memd(r7+#CONTEXT_r2524)
	}
	{
		r23:22 = memd(r7+#CONTEXT_r2322)
		r21:20 = memd(r7+#CONTEXT_r2120)
	}
	{
		r19:18 = memd(r7+#CONTEXT_r1918)
		r17:16 = memd(r7+#CONTEXT_r1716)
		crswap(r7,sgp0)
	}
	{
		rte  // ??? AXOK?
	}

		// call H2K_fatal_thread or H2K_fatal_kernel
	.falign
.Lfatal:
	{
		p0 = tstbit(r4,#18)  // interrupts enabled?
		r1:0 = combine(r7,#-1)
		r3:2 = s31:30  // pcycle
	}
	{
		r4 = htid
		if (p0) jump ##H2K_fatal_thread  // interrupts enabled, regular thread
	}
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
		r0 = sgp0
		jump H2K_thread_id // 1
	}
	{
		r2 = sgp0
		jump H2K_futex_wait // 2
	}
	{
		r2 = sgp0
		jump H2K_futex_resume // 3
	}
	{
		r5 = sgp0
		jump H2K_thread_create // 4
	}
	{
		r0 = sgp0
		jump H2K_thread_stop // 5
	}
	{
		r0 = sgp0
		jump H2K_cputime_get // 6
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 7 // jumpr r31?
	}
	{
		r2 = sgp0
		jump H2K_register_fastint // 8
	}
	{
		r2 = sgp0
		jump H2K_prio_set // 9
	}
	{
		r1 = sgp0
		jump H2K_prio_get // 10
	}
	{
		r1 = sgp0
		jump H2K_popup_wait	// 11
	}
	{
		r0 = sgp0
		jump H2K_sched_yield // 12
	}
	{
		r0 = sgp0
		jump H2K_thread_id	// 13
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 14
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 15
	}
	{
		r0 = sgp0
		jump H2K_pcycles_get // 16
	}
	{
		//r1 = sgp0
		//jump H2K_set_prefetch // 17
		r0 = sgp0
		jump H2K_thread_id
	}
	{
		r1 = sgp0
		jump H2K_tid_set // 18
	}
	{
		r0 = sgp0
		jump H2K_tid_get // 19
	}
	{
		r1 = sgp0
		jump H2K_futex_lock_pi // 20
	}
	{
		r1 = sgp0
		jump H2K_futex_unlock_pi // 21
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 22
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 23
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 24
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 25
	}
	{
		r0 = sgp0
		jump H2K_thread_id // 26
	}
	{
		r1 = sgp0
		jump H2K_waitcycles_get // 27
	}
	{
		r5 = sgp0
		jump H2K_vmboot // 28
	}
	{
		r4 = sgp0
		jump H2K_trap_pmuconfig // 29
	}
	{
		r5 = sgp0
		jump H2K_trap_config // 30
	}
	{
		r4 = sgp0
		jump H2K_trap_hwconfig // 31
	}
FUNC_END H2K_traptab

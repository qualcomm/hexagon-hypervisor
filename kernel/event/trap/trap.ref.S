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
	crswap(r0,sgp0)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* R7 = sgp0, r8 = ssr, r9 = htid */
	r19 = and(r8,#0xff)
	p0 = cmp.gt(r19,#0x1f)
	if (p0) jump 1f					// no, too big.
	r18 = memw(r7+#CONTEXT_trapmask)
	p0 = tstbit(r18,r19)
	if (!p0) jump 1f				// no, disabled.

	r18.h = #hi(H2K_traptab)
	r18.l = #lo(H2K_traptab)
	r18 = addasl(r18,r19,#3)

	// set up continuation in case we block
	call .Ljump_to_trap

H2K_trap_done:
	r17 = sgp0
	memd(r17+#CONTEXT_r0100) = r1:0		// Save return value in case of preemption
	call H2K_preempt			// Try to take interrupt before returning

H2K_guest_return:
	jump H2K_context_restore_return

1:
	memw(r7+#CONTEXT_continuation) = r31
	r0 = #0
	r1 = r19
	r2 = #TRAP0_GEVB_OFFSET
	r3 = r7
	call H2K_vm_event
	call H2K_preempt
	jump H2K_context_restore_return

.Ljump_to_trap:
	memw(r7+#CONTEXT_continuation) = r31
	jumpr r18

	/* EJP: all context carefully saved */
	/* We have a disabled or other trap failure, return to guest */
	/* GEVB is in r19 */
	/* Preds are in R18 */
	/* r17:16 == ssr:elr */
FUNC_END H2K_handle_trap0

FUNC_START H2K_traptab .text.core.trap0 .p2align 8
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
		r1 = r7
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
		r1 = r7
		jump H2K_popup_wait	// 11
	}
	{
		r0 = r7
		jump H2K_sched_yield // 12
	}
	{
		r0 = r7
		jump H2K_thread_id	// 13
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
		r0 = r7
		jump H2K_thread_id // 17
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
		r4 = r7
		jump H2K_timer_trap // 22
	}
	{
		r1 = r7
		jump H2K_fatal_crash // 23
	}
	{
		r4 = r7
		jump H2K_tlb_tlbop // 24
	}
	{
		r2 = r7
		jump H2K_thread_state // 25
	}
	{
		r1 = r7
		jump H2K_trap_info // 26
	}
	{
		r1 = r7
		jump H2K_waitcycles_get // 27
	}
	{
		nop
		jump .Lvmop_stub // 28
	}
	{
		r4 = r7
		jump H2K_trap_pmuctrl // 29
	}
	{
		r5 = r7
		jump H2K_trap_config // 30
	}
	{
		r4 = r7
		jump H2K_trap_hwconfig // 31
	}

.Lvmop_stub:
	{
		memw(r29 + #-8) = r7
		r29 = add(r29, #-8)
		jump H2K_vmop
	}

FUNC_END H2K_traptab


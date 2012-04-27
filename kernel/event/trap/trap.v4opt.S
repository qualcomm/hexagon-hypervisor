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
		crswap(r0,sgp0)
	}
	{
		memd(r0+#CONTEXT_r3130) = r31:30
		call H2K_context_save
	}
	/* sgp=r7, ssr=r8, htid=r9 */
	{
		r27 = and(r8,#0xff)
		r24 = memw(r7+#CONTEXT_trapmask)
	}
	{
		p0 = !cmp.gtu(r27,#0x1f)
		p0 = tstbit(r24,r27)
		if (!p0.new) jump:nt .Lreturn_to_guest
		r24 = r27
	}
	{
		r24 = add(##H2K_traptab,asl(r24,#3))
		call .Ljump_to_trap
	}
	.falign
H2K_trap_nogood:
	r17 = sgp
	{
		memd(r17+#CONTEXT_r0100) = r1:0
		call H2K_preempt
	}
	jump H2K_context_restore_return

	.falign
.Ljump_to_trap:
	{
		memw(r7+#CONTEXT_continuation) = r31  // set up continuation in case we block  ??? not tested?
		jumpr r24
	}

	.falign
.Lreturn_to_guest:
	{
		r1:0 = combine(r27,#0)
		r3:2 = combine(r7,#TRAP0_GEVB_OFFSET)
		call H2K_vm_event
		memw(r7+#CONTEXT_continuation) = r31
	}
	call H2K_preempt
	jump H2K_context_restore_return

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
		r4 = sgp0
		jump H2K_timer_trap // 22
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

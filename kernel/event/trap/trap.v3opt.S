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
	crswap(r0,sgp)
	{
		memd(r0+#CONTEXT_r3130) = r31:30
		call H2K_context_save
	}
	/* sgp=r7, ssr=r8, htid=r9 */
	{
		r27 = and(r8,#0xff)  // extract cause code
		r24 = memw(r7+#CONTEXT_trapmask)
		r17.h = #hi(H2K_traptab)
	}
	{
		p0 = cmp.gt(r27,#0x1f)  // trap too big?
		p1 = tstbit(r24,r27)  // trap enabled?
		r17.l = #lo(H2K_traptab)
	}
	{
		p0 = or(p0,!p1)
		r10 = #0
		if (p0.new) jump:t .Lreturn_to_guest  // no, too big or disabled.
	}
		/* OK to clobber caller saves */
	{
		r17 += asl(r27,#3)
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
		H2K_GP.l = #lo(H2K_kg)
		memw(r7+#CONTEXT_continuation) = r31  // set up continuation in case we block  ??? not tested?
		jumpr r17
	}

	.falign
.Lreturn_to_guest:
	{
		r1:0 = combine(r27,r10)
		r2 = #TRAP0_GEVB_OFFSET
		r3 = r7
		call H2K_vm_event
	}
	jump H2K_context_restore_return

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
		r1 = sgp
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
		r1 = sgp
		jump H2K_popup_wait	//11
	}
	{
		r0 = sgp
		jump H2K_sched_yield // 12
	}
	{
		r0 = sgp
		jump H2K_thread_id	// 13
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
		r4 = sgp
		jump H2K_timer_trap // 22
	}
	{
		r0 = sgp
		jump H2K_thread_id // 23
	}
	{
		r4 = sgp
		jump H2K_tlb_tlbop // 24
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
		r1 = sgp
		jump H2K_waitcycles_get // 27
	}
	{
		nop
		jump .Lvmop_stub  // 28
	}
	{
		r4 = sgp
		jump H2K_trap_pmuctrl // 29
	}
	{
		r5 = sgp
		jump H2K_trap_config // 30
	}
	{
		r4 = sgp
		jump H2K_trap_hwconfig // 31
	}

.Lvmop_stub:
	{
		memw(r29 + #-8) = r7  // == sgp
		r29 = add(r29, #-8)
		jump H2K_vmop
	}

FUNC_END H2K_traptab

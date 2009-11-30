/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

/*
 * optimization note:
 * 
 * The first two cache lines of the context should be prefetched.
 * The next several cache lines of the context should be DCZERO'd.
 */

FUNC_START BLASTK_handle_trap0 .text.event.trap0 .p2align 2
	r7 = sgp
	// dcfetch(r7+#0)
	// dcfetch(r7+#32)
	r8 = elr
	r9 = ssr
	// Detect ANGEL cal
	p0 = bitsclr(r9,#0x1f)
	if (p0) jump BLASTK_trap_angel
	// Save trap state
	memd(r7+#CONTEXT_r3130) = r31:30
	memd(r7+#CONTEXT_r2928) = r29:28
	memd(r7+#CONTEXT_r2726) = r27:26
	memd(r7+#CONTEXT_r2524) = r25:24
	memd(r7+#CONTEXT_r2322) = r23:22
	memd(r7+#CONTEXT_r2120) = r21:20
	memd(r7+#CONTEXT_r1918) = r19:18
	memd(r7+#CONTEXT_r1716) = r17:16
	memd(r7+#CONTEXT_ssrelr) = r9:8
	r13 = ugp
	r12 = gp
	memd(r7+#CONTEXT_ugpgp) = r13:12

	// Can we take this trap?
	r10 = and(r9,#0x1f)
	r11 = memw(r7+#CONTEXT_trapmask)
	p0 = tstbit(r11,r10)
	if (!p0) jump 1f				// no, kill thread or kernel

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(BLASTK_stacks)
	r29.l = #lo(BLASTK_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r29 += mpyi(r28,#KERNEL_STACK_SIZE)
#else
	r28 = TNUM
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##BLASTK_stacks,mpyi(r28,r29))
#endif

	r11.h = #hi(BLASTK_traptab)
	r11.l = #lo(BLASTK_traptab)
	r10 = addasl(r11,r10,#3)
	callr r10

blast_trap_nogood:
	r7 = sgp
	{
		r13:12 = memd(r7+#CONTEXT_ugpgp)
		r31:30 = memd(r7+#CONTEXT_r3130)
	}
	{
		r29:28 = memd(r7+#CONTEXT_r2928)
		r9:8 = memd(r7+#CONTEXT_ssrelr)
		ugp = r13
	}
	gp = r12
	ssr = r9
	elr = r8
9:
	rte

1:
	// call BLASTK_fatal_thread or BLASTK_fatal_kernel
	r0 = #-1
	r1 = r7
	r3 = pcyclehi
	r2 = pcyclelo
	r4 = ssr
	r4 = extractu(r0,#3,#19)
	p0 = tstbit(r4,#18)			// interrupts enabled?
	if (!p0) jump 1f
	jump BLASTK_fatal_thread		// no, regular thread
1:
	jump BLASTK_fatal_kernel		// yes, fast interrupt

FUNC_END BLASTK_handle_trap0

FUNC_START BLASTK_trap_angel .text.event.trap0 .p2align 2
	jump 9b
	.size BLASK_trap_angel, .-BLASTK_trap_angel
FUNC_END BLASTK_trap_angel

FUNC_START BLASTK_traptab .text.event.trap0 .p2align 8
	{
		jump BLASTK_trap_angel // 0
		nop
	}
	{
		r2 = r7
		jump BLASTK_thread_id // 1
	}
	{
		r2 = r7
		jump BLASTK_futex_wait // 2
	}
	{
		r2 = r7
		jump BLASTK_futex_resume // 3
	}
	{
		r5 = r7
		jump BLASTK_thread_create // 4
	}
	{
		r0 = r7
		jump BLASTK_thread_stop // 5
	}
	{
		r0 = r7
		jump BLASTK_cputime_get // 6
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 7 // jumpr r31?
	}
	{
		r2 = r7
		jump BLASTK_register_fastint // 8
	}
	{
		r2 = r7
		jump BLASTK_prio_set // 9
	}
	{
		r1 = r7
		jump BLASTK_prio_get // 10
	}
	{
		//r2 = r7
		//jump BLASTK_change_asid_mappings // 11
		r0 = r7
		jump BLASTK_thread_id
	}
	{
		r0 = r7
		jump BLASTK_sched_yield // 12
	}
	{
		//r1 = r7
		//jump BLASTK_mask_prios_above // 13
		r0 = r7
		jump BLASTK_thread_id
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 14
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 15
	}
	{
		r0 = r7
		jump BLASTK_pcycles_get // 16
	}
	{
		//r1 = r7
		//jump BLASTK_set_prefetch // 17
		r0 = r7
		jump BLASTK_thread_id
	}
	{
		r1 = r7
		jump BLASTK_tid_set // 18
	}
	{
		r0 = r7
		jump BLASTK_tid_get // 19
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 20
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 21
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 22
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 23
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 24
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 25
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 26
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 27
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 28
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 29
	}
	{
		r4 = r7
		jump BLASTK_trap_config // 30
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 31
	}
FUNC_END BLASTK_traptab


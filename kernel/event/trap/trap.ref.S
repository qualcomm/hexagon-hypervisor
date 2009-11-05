/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.global BLASTK_handle_trap0
BLASTK_handle_trap0:
	r7 = sgp
	r8 = elr
	r9 = ssr
	{
		memd(r7+#CONTEXT_ssrelr) = r9:8
		r13 = ugp
		p0 = bitsclr(r9,#0x3f)
		r11.h = #hi(traptab)
	}
	{
		if (!p0) memd(r7+#CONTEXT_r2928) = r29:28
		if (p0) jump BLASTK_trap_angel		// angel, abort!
		r12 = gp
		r16 = #TRAP_WONTBLOCK_MASK
	}
	{
		memd(r7+#CONTEXT_ugpgp) = r13:12
		r29 = add(r7,#THREAD_CONTEXT_TOTALSIZE)
		r11.l = #lo(traptab)
		p1 = tstbit(r9,#18)
	}
	{
		r15 = zxtb(r9)
		if (!p1) jump 1f
		r11 = tableidxd(r9,#5,#0)
	}
2:
	{
		callr r11
		memd(r7+#CONTEXT_r3130) = r31:30
	}
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

1:	// trap from fastint
	{
		p0 = tstbit(r16,r15)
		if (p0.new) jump:t 2b	// OK, won't block
	}
	r1 = badva
	r3 = ssr
	{
		r0 = r8
		r2 = #0xff
		r3 = extractu(r3,#3,#19)
	}
	r5 = pcyclehi
	r4 = pcyclelo
	r29.h = #hi(BLASTK_idle_stacks+16)
	r29.l = #lo(BLASTK_idle_stacks+16)
	jump BLASTK_error
	.size BLASTK_handle_trap0, .-BLASTK_handle_trap0

	.global BLASTK_trap_angel
BLASTK_trap_angel:
	jump 9b
	.size BLASK_trap_angel, .-BLASTK_trap_angel

	.p2align 8 // 256bytes ... 2x4 bytes x 32 entries
traptab:
	{
		jump BLASTK_trap_angel // 0
		nop
	}
	{
		r2 = r7
		jump BLASTK_interrupt_waitloop // 1
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
		jump BLASTK_trap_cputime // 6
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
		r2 = r7
		jump BLASTK_change_asid_mappings // 11
	}
	{
		r0 = r7
		jump BLASTK_sched_yield // 12
	}
	{
		r1 = r7
		jump BLASTK_mask_prios_above // 13
	}
#ifdef ERROR_HANDLING
	{
		r0 = r7
		jump BLASTK_error_get // 14
	}
	{
		r1 = r7
		jump BLASTK_restart_thread // 15
	}
#else
	{
		r0 = r7
		jump BLASTK_thread_id // 14
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 15
	}
#endif
	{
		r0 = r7
		jump BLASTK_get_pcycles // 16
	}
	{
		r1 = r7
		jump BLASTK_set_prefetch // 17
	}
	{
		r1 = r7
		jump BLASTK_set_tid // 18
	}
	{
		r0 = r7
		jump BLASTK_get_tid // 19
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
		r0 = r7
		jump BLASTK_thread_id // 30
	}
	{
		r0 = r7
		jump BLASTK_thread_id // 31
	}


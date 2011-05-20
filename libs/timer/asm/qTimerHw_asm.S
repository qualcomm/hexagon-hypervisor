/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	
.equ MATCH_OFFSET, 0
.equ COUNT_OFFSET, 4
.equ ENABLE_OFFSET, 8
.equ CLEAR_OFFSET, 12

.equ QDSP6_QTIMER_BASE, 0x28880000
.equ QTIMER_TEST_COUNT, 0xFFFFFFF0

	//void hw_timer_init (qtimetick_word_t match)
	.global hw_timer_init
	.type   hw_timer_init, @function
hw_timer_init:
		r2.h = #hi(QDSP6_QTIMER_BASE)
		r2.l = #lo(QDSP6_QTIMER_BASE)
	{
		//r2 = memw (#QDSP6_QTIMER_BASE)
		r2 = memw(r2)
		r1 = #0
		r3.h = #hi(QTIMER_TEST_COUNT)
	}
	{
		/* Disable Timer Hardware */
		memw (r2 + #ENABLE_OFFSET) = r1
		r1 = #1
		r3.l = #lo(QTIMER_TEST_COUNT)
	}
#ifndef QTIMER_ROLLOVER_TEST
	/* Reset the timer count */
	memw (r2 + #CLEAR_OFFSET) = r1
#else /* QTIMER_ROLLOVER_TEST */
	memw (r2 + #COUNT_OFFSET) = r3
1:
	r1 = memw (r2 + #COUNT_OFFSET)
	{
		p0 = cmp.eq (r1, r3)
		if (!p0.new) jump:nt 1b
	}
#endif /* QTIMER_ROLLOVER_TEST */
	/* Reset the Match Value to Maximum */
	{
		memw (r2 + #MATCH_OFFSET) = r0
		r1 = #1
	}
	/* Enable Timer Hardware but clear CLR_ON_MATCH_EN */
	memw (r2 + #ENABLE_OFFSET) = r1
	jumpr r31
	.size hw_timer_init, .-hw_timer_init

	//qtimetick_word_t hw_timer_curr_timetick ( void )
	.global hw_timer_curr_timetick
	.type   hw_timer_curr_timetick, @function
	.p2align 5 // align to cache line (32 bytes)
hw_timer_curr_timetick:
	r2.h = #hi(QDSP6_QTIMER_BASE)
	r2.l = #lo(QDSP6_QTIMER_BASE)
	//r2 = memw (#QDSP6_QTIMER_BASE)
	r2 = memw(r2)
	r0 = memw (r2 + #COUNT_OFFSET)
1:
	r1 = memw (r2 + #COUNT_OFFSET)
	{
		p0 = cmp.eq (r0, r1)
		if (!p0.new) jump:nt 1b
		r0 = r1
	}
	jumpr r31
	.size hw_timer_curr_timetick, .-hw_timer_curr_timetick

	//qtimetick_word_t hw_timer_match_val ( void )
	.global hw_timer_match_val
	.type   hw_timer_match_val, @function
hw_timer_match_val:
	r2.h = #hi(QDSP6_QTIMER_BASE)
	r2.l = #lo(QDSP6_QTIMER_BASE)
	r2 = memw(r2)
	{
		r0 = memw (r2 + #MATCH_OFFSET)
		jumpr r31
	}
	.size hw_timer_curr_timetick, .-hw_timer_curr_timetick

	//qtimetick_word_t hw_timer_prg_next_interrupt (qtimetick_word_t match, qtimetick_word_t count);
	.global hw_timer_prg_next_interrupt
	.type   hw_timer_prg_next_interrupt, @function
	.p2align 5 // align to cache line (32 bytes)
hw_timer_prg_next_interrupt:
	r2.h = #hi(QDSP6_QTIMER_BASE)
	r2.l = #lo(QDSP6_QTIMER_BASE)
	//r2 = memw (#QDSP6_QTIMER_BASE)
	r2 = memw(r2)
	r3 = memw (r2 + #COUNT_OFFSET)
	r4 = sub (r3, r1)
	{
		p0 = cmp.gt (r4, #1)
		if (p0.new) jump:nt 1f
	}
	{
		memw (r2 + #MATCH_OFFSET) = r0
	}
1:
	{
		r0 = r4
		jumpr r31
	}
	.size hw_timer_prg_next_interrupt, .-hw_timer_prg_next_interrupt


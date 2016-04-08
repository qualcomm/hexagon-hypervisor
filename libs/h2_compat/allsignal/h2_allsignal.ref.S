/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

#define WAITING 0
#define SIGNALS_IN 4
	.text

	/* 
	 * h2_signal_wait(h2_signal_t *signal, unsigned int mask)
	 * if each bit set in mask has already been seen, clear from signals_in and return.
	 * Else, set waiting mask with unseen bits in signal struct and sleep.
	 */
FUNC_START h2_allsignal_wait .text.h2.allsignal .falign
	/* Write wait mask */
	{
		r2 = memw_locked(r0)
	}
	{
		memw_locked(r0,p0) = r1
	}
	{
		if (!p0) jump h2_allsignal_wait		/* Not atomic update of signal (??) */
		r4 = add(r0,#SIGNALS_IN)
		r5 = not(r1)
	}
	/* Clear matching bits out of signals_in */
1:
	{
		r3 = memw_locked(r4)
	}
	{
		r6 = and(r3,r5)
		r7 = not(r3)
	}
	{
		memw_locked(r4,p0) = r6
	}
	if (!p0) jump 1b
	/* Clear those bits in wait mask */
1:
	{
		r2 = memw_locked(r0)
	}
	{
		r2 = and(r7,r2)
	}
	{
		memw_locked(r0,p0) = r2
	}
	if (!p0) jump 1b
	{
		p0 = cmp.eq(r2,#0)
	}
	{
		if (p0) jumpr r31
	}
	{
		memd(r29+#-8) = r25:24
		r29 = add(r29,#-8)
		r25:24 = combine(r1,r0)
		r1 = r2
	}
	/* Wait until our bits are cleared */
1:
	trap0(#H2_TRAP_FUTEX_WAIT)
	r1 = memw(r24)
	{
		p0 = cmp.eq(r1,#0)
		if (!p0.new) jump:nt 1b	// should not happen
		if (!p0.new) r0 = r24
	}
	{
		r1:0 = combine(r25,r24)
		r25:24 = memd(r29++#8)
		jumpr r31
	}
FUNC_END h2_allsignal_wait

	/* 
	 * h2_allsignal_signal(h2_signal_t *signal, unsigned int mask)
	 * 
	 * 
	 * 
	 */
FUNC_START h2_allsignal_signal .text.h2.allsignal .falign
	{
		r4 = not(r1)
		r5 = add(r0,#SIGNALS_IN)
	}
1:
	{
		r2 = memw_locked(r0)
	}
	{
		r3 = and(r2,r4)
		p1 = cmp.eq(r2,#0)
		if (p1.new) jump:nt 2f			// nothing to wait for, add to signals_in
	}
	{
		memw_locked(r0,p0) = r3
	}
	{
		if (!p0) jump 1b			// not atomic, try again
		p0 = cmp.eq(r3,#0)			// all bits cleared?
		if (p0.new) r1 = #1
	}
	{
		if (!p0) jumpr r31			// nope, return
	}
	trap0(#H2_TRAP_FUTEX_WAKE)
	jumpr r31
2:
	/* nobody is waiting on anything, so add to signals_in to be observed later */
	r2 = memw_locked(r5)
	r2 = or(r2,r1)
	memw_locked(r5,p0) = r2
	if (p0) jumpr r31				// nothing else to do
	jump 1b						// not atomic, start all over 
FUNC_END h2_allsignal_signal


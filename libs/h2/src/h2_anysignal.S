/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

#define WAITING 4
#define SIGNALS 0
	.text

	.global h2_anysignal_wait
	/* 
	 * h2_anysignal_wait(h2_signal_t *signal, unsigned int mask)
	 * Wait for one of the signals specified in mask is set
	 * If already set, return immediately, otherwise block
	 * return value is latest value of signals
	 */
h2_allsignal_wait:
	/* Check current value */
	{
		r4 = memw(r0+#SIGNALS)		// get SIGNALS
		memw(r0+#WAITING) = r1		// set WAITING
	}
	{
		p0 = bitsclr(r4,r1)		// Are all WAITING bits clear in SIGNALS?
		if (!p0.new) r0 = r1		// No, return val is new signals
		if (!p0.new) jump:nt 9f		// no, return
	}
	/* All WAITING bits are clear, which means we need to block */
	{
		allocframe(#8)
		r1 = r4
	}
	{
		memd(r29) = r25:24
		r25:24 = combine(r1,r0)
	}
1:
	trap0(#H2_TRAP_FUTEX_WAIT)
	r3:2 = memd(r24+#0)
	{
		p0 = bitsclr(r0,r1)		// are all WAITING bits clear in SIGNALS?
		if (p0.new) r1:0 = combine(r25,r24)
		if (p0.new) jump:nt 1b		// got spurrious wakeup, go back to sleep
	}
	{
		// One of the signals has been set, newest signals in r0 (return val)
		// restore ABI stuff and return
		r25:24 = memd(r29)		// restore callee save regs
		deallocframe			// restore stack frame
	}
9:
	jumpr r31				// return

	.global h2_anysignal_set
	/* 
	 * h2_anysignal_set(h2_signal_t *signal, unsigned int mask)
	 * Set signals defined in mask
	 * This may wakeup a waiting thread, check for bitsclear->!bitsclr condition
	 * 
	 */
h2_anysignal_set:
1:
	{
		r2 = memw_locked(r0)		// load old signals
	}
	{
		r4 = or(r2,r1)			// OR in new signals
		r3 = memw(r0+#WAITING)
	}
	{
		memw_locked(r0,p0) = r4		// save new signals
	}
	{
		p1 = bitsclr(r4,r3)		// new value has matching signal?
		p2 = bitsclr(r2,r3)		// old value has matching signal?
	}
	{
		if (!p0) jump 1b		// not atomic, try again
		p3 = and(p1,!p2)		// Old value not matching, new matching
	}
	{
		if (!p3) jumpr r31		// Still not matching, no wakeup
		if (!p3) r0 = r2		// set old signals as return val
	}
	{
		allocframe(#8)
		r1 = #1				// wake up 1 thread??
	}
	{
		memw(r29) = r2			// return value: old signals
	}
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		deallocframe
		r0 = memw(r29)			// return value: old signals
	}
	jumpr r31

	.global h2_anysignal_clear
	/* 
	 * unsigned int h2_anysignal_clear(h2_allsignal_t *signal, unsigned int mask)
	 * clear bits out of signal
	 * return old signal value
	 */
h2_anysignal_clear:
	r3 = not(r1)
1:
	r2 = memw_locked(r0)
	r4 = and(r3,r2)
	memw_locked(r0,p0) = r4
	if (!p0) jump 1b
	{
		r0 = r2
		jumpr r31
	}


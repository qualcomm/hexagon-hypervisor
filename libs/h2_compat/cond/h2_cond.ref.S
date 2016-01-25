/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_cond.S
 * 
 * This file contains pthreads-like condition variable implementation 
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

	.text
	//.global h2_cond_wait_rmutex

/*
 * About the implementation of the condition variables:
 *  We have two parts: "count" and "n_waiting".
 *  "n_waiting" is incremented by waiting threads and 
 *       decremented when a thread wakes.  It prevents
 *       unnecessary calls to FUTEX_WAKE
 *  "count" is just a counter that should wrap around.
 *       It is incremented on every signal/broadcast,
 *       so that we catch WAKE/WAIT race conditions, and
 *       potentially some spurious wakeups.
 *
 */

/*
 * Internally, we use "h2_cond_signal_n", which wakes up
 * n threads max.  We add an appropriate value for the two 
 * calls.
 */

FUNC_START h2_cond_broadcast .text.h2.cond .p2align 5
	{
		r1 = #0x7fff
		jump .Lh2_cond_signal_n
	}
FUNC_END h2_cond_broadcast

FUNC_START h2_cond_signal .text.h2.cond .p2align 4
	{
		r1 = #1
	}
.Lh2_cond_signal_n:
	{
		r5 = #1				// set up some constants
		allocframe(#8)
		memd(r29+#-16) = r17:16
		r17:16 = r1:0
	}
1:
	r2 = memw_locked(r0)
	{
		r3 = vaddh(r5,r2)		// add to counter
		r4 = asrh(r2)			// upper half (n_waiting)
	}
	memw_locked(r0,p0) = r3			// set bumped counter
	{
		if (!p0) jump 1b		// not atomic
		p1 = cmp.eq(r4,#0)		// nobody waiting?
	}
	{
		if (p1) jumpr r31			// if nobody waiting, leave
	}
.Ldo_trap:
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		p0 = cmp.gt(r0,#-1)
		if (p0.new) r17:16 = memd(r29)
		if (p0.new) dealloc_return:t
	}
	{
		r1:0 = r17:16
		r2 = memw(r16)
		jump .Ldo_trap
	}
FUNC_END h2_cond_signal

	/* 
	 * Condition Wait
	 * - Read the condition variable
	 * - Unlock the mutex
	 * - Bump n_waiting in the condition variable, check that count matches old read
	 * - Block
	 * - When unblocked, decrement n_waiting
	 * - Acquire lock
	 * - return
	 * 
	 * Note: in Linux, we have a call to requeue the threads waiting on the 
	 * condition var to wait on the mutex.  As it is now, most of these threads
	 * will wake up and go right back to sleep waiting on the mutex.  Oh well.
	 */
FUNC_START h2_cond_wait .text.h2.cond .falign
	{
		allocframe(#16)
		r3:2 = combine(#1,#0)
	}
	{
		memd(r29+#0) = r25:24
		r24 = memw(r0)
		r25 = combine(r3.l,r2.l)		// 0x00010000 for addition
	}
	{
		memd(r29+#8) = r27:26
		r27:26 = combine(r1,r0)
		r0 = r1
		call h2_mutex_unlock
	}
1:
	r0 = memw_locked(r26)
	{
		p1 = vcmph.eq(r25:24,r1:0)		// we care about lowest halfword
		if (!p1.new) jump:nt 8f			// while unlocking, count changed
		r1 = vadduh(r0,r25):sat			// bump threads waiting
	}
	memw_locked(r26,p0) = r1			// write new value
	{
		if (!p0) jump 1b			// not atomic, try again
		r0 = r26				// futexptr for trap
	}
2:
	trap0(#H2_TRAP_FUTEX_WAIT)
1:							// Now, decrement n_waiting
	r0 = memw_locked(r26)
	{
		r3 = vsubuh(r0,r25):sat			// bump threads waiting
		p1 = vcmph.eq(r25:24,r1:0)		// we care about lowest halfword
		if (p1.new) jump:nt 2b			// count didn't change, temporary fail? block again
		if (p1.new) r1:0 = combine(r0,r26)
	}
	memw_locked(r26,p0) = r3			// write new value
	if (!p0) jump 1b				// not atomic, try again
8:							// lock and return
	{
		r25:24 = memd(r29+#0)
		r27:26 = memd(r29+#8)
		r0 = r27
	}
	{
		deallocframe
		jump h2_mutex_lock
	}
FUNC_END h2_cond_wait


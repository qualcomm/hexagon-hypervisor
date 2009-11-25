/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_mutex.S
 * 
 * This file contains the implementation for 
 * blast_mutex functionality 
 */

#include "blast_trap_constants.h"

	.text
	.global blast_mutex_lock
	.global blast_mutex_unlock
	.global blast_mutex_trylock

	/* blast_mutex_lock
 	 * if lock is:
	 * 0 we turn it to 1 and return
	 * 1 we turn it to 2 and wait
	 * 2 we keep it at 2 and wait
	 * Once resuming from a wait, we try again, if the lock is:
	 * 0 we turn it to 2 and return
	 * 1 we turn it to 2 and wait
	 * 2 we turn it to 2 and wait
	 * 
	 * Our strategy:
	 * At the beginning of the function, set <lockedval> to 1
	 * Then we have our decision:
	 * If lock is:
	 * 0 we turn it to <lockedval>
	 * 1 we turn it to 2 and wait
	 * 2 we turn it to 2 and wait
	 * After the first lock attempt, we set <lockedval> to 2,
	 * and then the decision logic works again
	 */
blast_mutex_lock:
	{
		r1 = #1				// counter
		r7 = #1				// start with <lockedval> == 1
	}
1:
	r2 = memw_locked(r0)
	{
		p0 = cmp.eq(r2,#0)
		p2 = cmp.eq(r2,#2)
	}
	{
		if (!p0) r7 = #2	// forever try 2 instead of 1
		if (!p0) jump 3f	// not available
	}
	memw_locked(r0,p1) = r7		// Swap in <lockedval>
	if (!p1) jump 1b 		// try again, not atomic
	jumpr r31			// we got the lock! r31 wasn't clobbered
3:
	memw_locked(r0,p1) = r7		// try to swap in a 2		// NOTE: v2 fails bceause of mispredict
	if (!p1) jump 1b	// wasn't atomic, try again
	allocframe(#8)
	{
		memd(r29+#0) = r1:0
		r1 = #2			// futex expected value
	}
	trap0(#BLAST_TRAP_FUTEX_WAIT)
	/* We've waited our turn, spin an increasing number of times */
	/* (Livelock prevention) */
	{
		r1:0 = memd(r29+#0)
		deallocframe
		r7 = #2			// restore <lockedval>
	}
	{
		r4 = r1			// get counter value
		r1 = add(r1,r1)		// double time to spin
	}
5:
	r2 = memw_locked(r0)
	{
		p0 = cmp.eq(r2,#0)	// lock freed?
		p1 = cmp.gt(r4,#0)	// Still time to spin?
		r4 = add(r4,#-1)	// decrement counter
		if (!p1.new) jump:nt 1b	// if no more time to spin, start over
	}
	{
		if (!p0) jump 5b	// continue to spin
	}
	memw_locked(r0,p1) = r7
	{
		if (!p1) jump 5b	// not atomic, spin again
	}
	// Yay!  We got the lock and the swap to 2 was atomic.
	// So... return!
	jumpr r31

	/*
	 * blast_mutex_unlock
	 * If lock is:
	 * 0: WTF? Already unlocked, I guess just return.
	 * 1: Nobody waiting in kernel, just unlock and return.
	 * 2: futex_wake one thread and return.
	 */
blast_mutex_unlock:
	r1 = #0
1:
	r2 = memw_locked(r0)
	memw_locked(r0,p0) = r1
	{
		if (!p0) jump 1b		// not atomic, try swap again
		p1 = cmp.eq(r2,#2)		// was someone asleep?
	}
	{
		if (!p1) jumpr r31		// nobody was alseep, we're good to go
		r1 = #1				// otherwise, we want to wake one thread
	}
	allocframe(#0)
	trap0(#BLAST_TRAP_FUTEX_WAKE)
	deallocframe
	jumpr r31

	/* 
	 * blast_mutex_trylock
	 * If mutex is 0, change it to 1 and return 0
	 * Otherwise, return nonzero
	 */
blast_mutex_trylock:
	r1 = #1
1:
	r2 = memw_locked(r0)
	{
		p0 = cmp.eq(r2,#0)
		if (!p0.new) jump:nt 2f
		if (!p0.new) r0 = #1
	}
	memw_locked(r0,p1) = r1
	{
		if (!p1) jump 1b
		if (p1) r0 = #0
	}
2:
	jumpr r31


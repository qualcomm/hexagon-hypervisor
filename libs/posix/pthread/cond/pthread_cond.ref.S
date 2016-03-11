/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * pthread_cond.S
 * 
 * This file contains pthreads-like condition variable implementation 
 */

#include <h2_common_asm_std.h>
#include <h2_trap_constants.h>

	.text

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
 * Internally, we use "pthread_cond_signal_n", which wakes up
 * n threads max.  We add an appropriate value for the two 
 * calls.
 */

FUNC_START pthread_cond_broadcast .text.pthread.cond .p2align 5
	{
		r1 = #0x7fff
		jump .Lpthread_cond_signal_n
	}
FUNC_END pthread_cond_broadcast

FUNC_START pthread_cond_signal .text.pthread.cond .p2align 4
	{
		r1 = #1
	}
.Lpthread_cond_signal_n:
#define PTR r0
#define N r1
#define N_PTR r1:0
#define CONDVAL r2
#define CONDVAL_NEW r3
#define N_WAITING r4
#define ONE r5
#define N_SAV r17
#define PTR_SAV r16
#define N_PTR_SAV r17:16
	{
		ONE = #1				// set up some constants
		allocframe(#8)
		memd(r29+#-16) = N_PTR_SAV
		N_PTR_SAV = combine(N,PTR)
	}
1:
	CONDVAL = memw_locked(PTR)
	{
		CONDVAL_NEW = vaddh(ONE,CONDVAL)	// add to counter
		N_WAITING = lsr(CONDVAL,#16)	// upper half (n_waiting)
	}
	memw_locked(PTR,p0) = CONDVAL_NEW	// set bumped counter
	{
		if (!p0) jump 1b		// not atomic
		p1 = cmp.eq(N_WAITING,#0)	// nobody waiting?
	}
	{
		if (p1) r0 = #0
		if (p1) N_PTR_SAV = memd(r29+#0)
		if (p1) dealloc_return		// if nobody waiting, leave
	}
.Ldo_trap:
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		p0 = cmp.gt(r0,#-1)
		if (p0.new) N_PTR_SAV = memd(r29)
		if (p0.new) dealloc_return:t
		if (p0.new) r0 = #0
	}
	{
		N_PTR = N_PTR_SAV
		jump .Ldo_trap
	}
#undef N
#undef PTR
#undef N_PTR
#undef CONDVAL
#undef CONDVAL_NEW
#undef N_WAITING
#undef ONE
#undef N_SAV
#undef PTR_SAV
#undef N_PTR_SAV
FUNC_END pthread_cond_signal

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
FUNC_START pthread_cond_wait .text.pthread.cond .falign
#define CONDPTR r0
#define MUTEXPTR r1
#define CONDPTR_SAV r17
#define MUTEXPTR_SAV r16
#define CONDPTR_MUTEXPTR_SAV r17:16
#define ADDVAL_SAV r18
#define CONDVAL_SAV r19
#define CONDVAL_ADDVAL_SAV r19:18
#define ONE_ZERO r3:2
#define CONDVAL r4
	{
		allocframe(#16)
		memd(r29+#-16) = CONDVAL_ADDVAL_SAV
	}
	{
		memd(r29+#0) = CONDPTR_MUTEXPTR_SAV
		CONDVAL_SAV = memw(CONDPTR)		// should duplex
		ADDVAL_SAV = ##0x00010000
	}
	{
		CONDPTR_MUTEXPTR_SAV = combine(CONDPTR,MUTEXPTR)
		r0 = MUTEXPTR
		call pthread_mutex_unlock
	}
1:
	CONDVAL = memw_locked(CONDPTR_SAV)
	{
		
		p1 = cmph.eq(CONDVAL_SAV,CONDVAL)	// we care about lowest halfword
		if (!p1.new) jump:nt 8f			// while unlocking, count changed
		CONDVAL = add(CONDVAL,ADDVAL_SAV)	// bump threads waiting
	}
	memw_locked(CONDPTR_SAV,p0) = CONDVAL		// write new value
	{
		if (!p0) jump 1b			// not atomic, try again
		r1:0 = combine(CONDVAL,CONDPTR_SAV)	// futexptr for trap
	}
2:
	trap0(#H2_TRAP_FUTEX_WAIT)
1:							// Now, decrement n_waiting
	CONDVAL = memw_locked(CONDPTR_SAV)
	{
		CONDVAL = sub(CONDVAL,ADDVAL_SAV)	// bump threads waiting
		p1 = cmph.eq(CONDVAL,CONDVAL_SAV)	// we care about lowest halfword
		if (p1.new) jump:nt 2b			// count didn't change, temporary fail? block again
		if (p1.new) r1:0 = combine(CONDPTR_SAV,CONDVAL)
	}
	memw_locked(CONDPTR_SAV,p0) = CONDVAL		// write new value
	if (!p0) jump 1b				// not atomic, try again
8:							// lock and return
	{
		CONDVAL_ADDVAL_SAV = memd(r29+#8)
	}
	{
		deallocframe
		CONDPTR_MUTEXPTR_SAV = memd(r29+#0)
		r0 = MUTEXPTR_SAV
		jump pthread_mutex_lock			// will return required 0
	}
FUNC_END pthread_cond_wait


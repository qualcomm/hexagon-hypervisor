/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_sem.S
 * 
 * This file contains the implementation for h2_sem functionality
 */

#include "h2_trap_constants.h"

	.text
	.global h2_sem_add
	.global h2_sem_down
	.global h2_sem_trydown

/* 
 * New H2 semaphore:
 * High half: # of threads waiting
 * Low half: Value of semaphore
 * H=0, L>0: available, nobody waiting
 * H>0, L>0: Available, one or more threads woken up, but haven't decremented H yet.
 *           Additional ups should continue to attempt to wake appropriate number of threads
 * H=0, L=0: Not available, nobody waiting, an up merely increments and returns
 * H>0, L=0: Not available, threads waiting.  Up needs to wake up threads.
 */

/*
 * h2_sem_add(word *sem,unsigned word K)
 * if sem is:
 * 0/>0: unlocked, nobody waiting, we add K to it and return
 * >0/>0: unlocked, someone waiting, we add K to it, futex_wake, and return
 * 0/0: locked, nobody waiting, we add K to it and return
 * >0/0: locked, somebody waiting, set to K, futex_wake, and return
 * How many threads to wake? 
 * If K==1, wake 1
 * Else, wake Everyone (assume: 30000 threads is everyone)! (Or K?)
 */

h2_sem_add:
	r4 = #-0x8000
1:
	r2 = memw_locked(r0)
	{
		p0 = bitsclr(r2,r4)		// top half zero? (one extra bit don't care)
		r3 = vadduh(r1,r2):sat		// add to semaphore value
	}
	memw_locked(r0,p2) = r3
	{
		if (!p2) jump 1b		// atomicity failed, start over
		p1 = cmp.eq(r1,#1)		// K==1?
		r2 = #30000
	}
	{
		if (p0) jumpr r31			// nobody was waiting, done
		if (!p1) r1 = r2			// If K != 1, wake 30000
		if (p1) r1 = #1				// else wake 1
		if (p0) r0 = r4
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r17:16
		r17:16 = r1:0
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
		r2 = memw(r16)				// put back in TLB
		jump .Ldo_trap
	}

/* 
 * h2_sem_trydown(word *sem)
 * 
 * atomic: if sem > 0, decrement sem; return 0.
 * Else, return nonzero
 *
 */

h2_sem_trydown:
	r4 = #0
1:
	r2 = memw_locked(r0)
	{
		p0 = vcmph.gtu(r3:2,r5:4)
		r3 = add(r2,#-1)
		if (!p0.new) jump:nt 1f
	}
	memw_locked(r0,p2) = r3
	{
		if (p2) jumpr r31
		if (p2) r0 = #0
	}
	if (!p2) jump 1b
1:
	// lock not available, return failure
	{
		r0 = #-1
		jumpr r31
	}

/*
 * h2_sem_down(word *sem)
 * if sem <= 0, make -1, goto "wait_seq"
 * atomic_decrement *sem.
 * If atomicity failed, start over
 * Otherwise, return
 * 
 * wait_seq:
 * futex_wait(sem,-1)
 * If sem > 1, decrement sem, return
 * If sem == 1, sem = -1, return
 * If sem <= 0, sem = -1, goto "wait_seq"
 */

h2_sem_down:
	r5:4 = combine(#1,#0)
1:
	r2 = memw_locked(r0)
	{
    		p0 = vcmph.gtu(r3:2,r5:4)	// only care about lsb
		r3 = add(r2,#-1)
		r1 = aslh(r5)			// 1:0 packed
	}
	{
		if (!p0) jump 4f		// move into prev packet w/ .new for v3
		r1 = vadduh(r2,r1):sat		// add 1 to asleep threads field
	}
	memw_locked(r0,p2) = r3
	if (p2) jumpr r31			// Hey, the sem_down was successful! Not bad!
	if (!p2) jump 1b			// not atomic, try again
4:
	// lock already held, need to wait
	memw_locked(r0,p2) = r1			// set thread asleep
	if (!p2) jump 1b			// not atomic, try again
2:
	{				// OK, sleep.  r0=lock, r1=n_asleep:0
		allocframe(#8)
		memd(r29+#-16) = r1:0
	}
	trap0(#H2_TRAP_FUTEX_WAIT)
	{
		deallocframe
		r1:0 = memd(r29)
		r5.h = #1
	}
	{
		r5.l = #1
		r4 = #0
	}
3:
	r2 = memw_locked(r0)
	{
    		p0 = vcmph.gtu(r3:2,r5:4)	// only care about lsb... was something avail?
		r3 = vsubuh(r2,r5):sat		// subtract 1 from sleeping, 1 from semaphore
		if (!p0.new) jump:nt 2b		// Lock wasn't available, go back to sleep.
		r1 = r2
	}
	memw_locked(r0,p2) = r3
	if (p2) jumpr r31			// atomic, return
	jump 3b					// not atomic, try again


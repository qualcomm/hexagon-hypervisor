/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_sem.S
 * 
 * This file contains the implementation for h2_sem functionality
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

	.text

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

/* FIXME: get errno into asm... */
#define EAGAIN 1
#define EOVERFLOW 1

FUNC_START pthread_sem_add_np .text.pthread.sem .p2align 5
#define SEMPTR r0
#define AMOUNT r1
#define SEMPTR_AMOUNT r1:0
#define MASK r4
#define SEMVAL r2
#define SEMVAL_NEW r3
#define MAXSEMVAL r5
#define TMP r6
#define SEMPTR_AMOUNT_SAV r17:16
#define SEMPTR_SAV r16
#define AMOUNT_SAV r17
	{
		MASK = #-0x8000
		MAXSEMVAL = sub(##0x7fff,AMOUNT)
	}
1:
	SEMVAL = memw_locked(SEMPTR)
	{
		p0 = bitsclr(SEMVAL,MASK)		// top half zero? (one extra bit don't care)
		p1 = cmph.gtu(SEMVAL,MAXSEMVAL)
	}
	{
		if (p1) jump .Lvalue_fail
		SEMVAL_NEW = vadduh(SEMVAL,AMOUNT):sat	// add to semaphore value
	}
	{
		memw_locked(SEMPTR,p2) = SEMVAL_NEW
	}
	{
		if (!p2) jump 1b		// atomicity failed, start over
	}
	{
		if (p0) jumpr r31			// nobody was waiting, done
		if (p0) r0 = #0
	}
	{
		allocframe(#8)
		memd(r29+#-16) = SEMPTR_AMOUNT_SAV
		SEMPTR_AMOUNT_SAV = SEMPTR_AMOUNT
	}
.Ldo_trap:
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		p0 = cmp.gt(r0,#-1)
		if (p0.new) SEMPTR_AMOUNT_SAV = memd(r29)
		if (p0.new) dealloc_return:t
	}
	{
		SEMPTR_AMOUNT = SEMPTR_AMOUNT_SAV
		TMP = memw(SEMPTR_SAV)				// put back in TLB
		jump .Ldo_trap
	}
.Lvalue_fail:
	{
		r0 = #EOVERFLOW
		jumpr r31
	}
#undef SEMPTR
#undef AMOUNT
#undef SEMPTR_AMOUNT
#undef MASK
#undef SEMVAL
#undef TMP
#undef SEMPTR_AMOUNT_SAV
#undef SEMPTR_SAV
#undef AMOUNT_SAV
FUNC_END pthread_sem_add_np

FUNC_START pthread_sem_wait_np .text.pthread.sem .falign
#define SEMPTR r0
#define SEMVAL r1
#define SEMVAL_NEW r3
#define ADDVAL r4
#define ZERO r5
#define SEMPTR_SAV r16
	{
		ADDVAL = ##0x00010000
		SEMVAL = memw_locked(SEMPTR)
	}
	{
    		p0 = cmph.eq(SEMVAL,#0)	// only care about lsb
		if (p0.new) jump:t .Lblock
		SEMVAL_NEW = add(SEMVAL,#-1)
	}
	memw_locked(SEMPTR,p2) = SEMVAL_NEW
	{
		if (p2) jumpr r31
		if (p2) r0 = #0
	}
	{
		jump pthread_sem_wait_np
	}
.Lblock:
	SEMVAL_NEW = add(ADDVAL,SEMVAL)
	memw_locked(SEMPTR,p2) = SEMVAL_NEW
	if (!p2) jump pthread_sem_wait_np
	{
		allocframe(#8)
		memw(r29+#-16) = SEMPTR_SAV
		SEMPTR_SAV = SEMPTR
		SEMVAL = SEMVAL_NEW
	}
.Ldo_block:
	trap0(#H2_TRAP_FUTEX_WAIT)
	{
		SEMVAL = memw_locked(SEMPTR_SAV)
		ADDVAL = ##0x00010001
		ZERO = #0
	}
	{
		SEMVAL_NEW=vsubuh(SEMVAL,ADDVAL):sat	// maybe just sub. Dec count + sleepers
		p0 = cmph.eq(SEMVAL,#0)
		if (p0.new) jump:nt .Ldo_block		// still not valid, go to sleep again
		SEMPTR = SEMPTR_SAV
	}
	memw_locked(SEMPTR_SAV,p0) = SEMVAL
	{
		r0 = mux(p0,#0,SEMPTR_SAV)
		if (p0) SEMPTR_SAV = memw(r29)
		if (p0) dealloc_return
	}
	{
		jump .Ldo_block
	}
#undef SEMPTR
#undef SEMVAL
#undef SEMVAL_NEW
#undef ADDVAL
#undef ZERO
#undef SEMPTR_SAV
FUNC_END pthread_sem_wait_np

/* 
 * h2_sem_trydown(word *sem)
 * 
 * atomic: if sem > 0, decrement sem; return 0.
 * Else, return nonzero
 *
 */

FUNC_START pthread_sem_trywait_np .text.pthread.sem .falign
#define SEMPTR r0
#define SEMVAL r2
#define SEMVAL_NEW r3
	SEMVAL = memw_locked(SEMPTR)
	{
		p0 = cmph.eq(SEMVAL,#0)
		SEMVAL_NEW = add(SEMVAL,#-1)
		if (p0.new) jump:nt 1f
	}
	memw_locked(SEMPTR,p0) = SEMVAL
	{
		if (p0) jumpr r31
		if (p0) r0 = #0
	}
	jump pthread_sem_trywait_np
1:
	// lock not available, return failure
	{
		r0 = #EAGAIN
		jumpr r31
	}
#undef SEMPTR
#undef SEMVAL
#undef SEMVAL_NEW
FUNC_END pthread_sem_trywait_np


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_barrier.S
 * 
 * This file contains the implementation for h2_barrier functionality
 */

#include <h2_common_asm_std.h>
#include <h2_trap_constants.h>
#include <pthread_barrier.h>

	/* barrier type
	 * typedef union {
	 * 	struct {
	 * 		unsigned short threads_left;
	 * 		unsigned short version;
	 * 		unsigned int threads_total;
	 * 	};
	 * 	unsigned long long int raw;
	 * } h2_barrier_t;
	 * 
	 * count is a non-saturating counter to distinguish events
	 * threads_left is the number of threads until the barrier releases
	 * threads_total is the number of threads to reset to
	 * 
	 */

	/* 
	 * H2 barrier wait:
	 * 
	 * h2_barrier_wait(h2_barrier_t *barrier)
	 * 
	 * Read first word (count:threads_left) (memw_locked)
	 * If threads_left == 1, 
	 *  + increment count
	 *  + reset threads_left to threads_total
	 *  + write back word (memw_locked)
	 *  + wake threads_total
	 *  + return H2_BARRIER_SERIAL_THREAD
	 * else,
	 *  + decrement threads_left
	 *  + write back word (memw_locked)
	 *  + while re-read of count matches first read,
	 *    - wait
	 *  + return H2_BARRIER_OTHER
	 *
	 * EJP: should we even really spin here?
	 *
	 */

FUNC_START pthread_barrier_wait .text.pthread.barrier .p2align 4
#define BARRIER_PTR r0
#define FUTEX_TRAP_ARG r1
#define SPINSUB r5
#define ONE r4
#define SPINSUB_ONE r5:4
#define VERSION r6
#define CHECK_VERSION r7
#define BARRIER r3:2
#define BARRIER_LO r2
#define BARRIER_HI r3
#define NEW_BARRIER_LO r6
#define BARRIER_LO_SAV r17
#define BARRIER_PTR_SAV r16
#define BARRIER_LO_PTR_SAV r17:16
	{
		BARRIER_HI = memw(BARRIER_PTR+#4)
		SPINSUB = ##0x10000
		ONE = #1
	}
.Latomic_retry:
	{
		BARRIER_LO = memw_locked(BARRIER_PTR)
	}
	{
		p0 = cmph.eq(BARRIER_LO,ONE)		// Is threads_left (low hword) 1?
		if (p0.new) jump:t .Llast_thread	// If threads_left == 1, we're the last thread
		if (p0.new) NEW_BARRIER_LO = add(SPINSUB,BARRIER_LO)	// incremnent version
		if (!p0.new) NEW_BARRIER_LO = add(BARRIER_LO,#-1)	// decrement threads
	}
	memw_locked(BARRIER_PTR,p0) = NEW_BARRIER_LO	// commit decremented count
	{
		if (!p0) jump .Latomic_retry
		CHECK_VERSION = lsr(NEW_BARRIER_LO,#16)
	}
.Lspin_again:
	{
		VERSION = memuh(BARRIER_PTR+#2)
		if (!cmp.eq(VERSION.new,CHECK_VERSION)) jump:nt .Ldone
	}
	pause(#4)
	{
		p1 = cmp.gtu(SPINSUB,BARRIER_HI)	// Is spin field zero?
		if (!p1.new) jump:t .Lspin_again	// if spin field is nonzero, (and not last) we spin
		if (!p1.new) BARRIER_HI = sub(BARRIER_HI,SPINSUB)
	}
	/* BLOCK */
	{
		allocframe(#8)
		memd(r29+#-16) = BARRIER_LO_PTR_SAV
		BARRIER_LO_PTR_SAV = combine(NEW_BARRIER_LO,BARRIER_PTR)
		FUTEX_TRAP_ARG = BARRIER_LO
	}
.Lblock_again:
	trap0(#H2_TRAP_FUTEX_WAIT)
	{
		BARRIER_LO = memw(BARRIER_PTR_SAV+#0)		// Get new version
		CHECK_VERSION = lsr(BARRIER_LO_SAV,#16)		// Get saved version
	}
	{
		VERSION = lsr(BARRIER_LO,#16)
	}
	{
		p0 = cmp.eq(VERSION,CHECK_VERSION)	// Is the version still the same?
		if (p0.new) jump:nt .Lblock_again	// If so, block again
		if (p0.new) r1:0 = combine(BARRIER_LO,BARRIER_PTR_SAV)	// with correct arguments
	}
	{
		BARRIER_LO_PTR_SAV = memd(r29+#0)		// restore callee save regs
		deallocframe
	}
	/* Version has changed, so we're done */
.Ldone:
	{
		r0 = #0
		jumpr r31
	}

.Llast_thread:
	NEW_BARRIER_LO = combine(NEW_BARRIER_LO.H,BARRIER_HI.L)
	memw_locked(BARRIER_PTR,p0) = NEW_BARRIER_LO
	if (!p0) jump .Latomic_retry
	{
		allocframe(#8)
		memw(r29+#-16) = BARRIER_PTR_SAV
		BARRIER_PTR_SAV = BARRIER_PTR
		FUTEX_TRAP_ARG = zxth(BARRIER_HI)
	}
.Lfutex_wake_again:
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		p0 = cmp.gt(r0,#-1)
		r1:0 = combine(BARRIER_PTR_SAV,#PTHREAD_BARRIER_SERIAL_THREAD)
		BARRIER_PTR_SAV = memw(r29+#0)
		if (p0.new) dealloc_return:t
	}
	/* FUTEX WAKE FAILED!!! */
	{
		r2 = memw(r1)		// reread barrier ptr
		r0 = r1			// put arg in right place
		jump .Lfutex_wake_again
	}
FUNC_END pthread_barrier_wait


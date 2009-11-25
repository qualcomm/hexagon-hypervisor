/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_barrier.S
 * 
 * This file contains the implementation for blast_barrier functionality
 */

#include "blast_trap_constants.h"
#define ASM 1
#include "blast_barrier.h"

	.text
	.global blast_barrier_wait

	/* barrier type
	 * typedef union {
	 * 	struct {
	 * 		unsigned short threads_left;
	 * 		unsigned short count;
	 * 		unsigned int threads_total;
	 * 	};
	 * 	unsigned long long int raw;
	 * } blast_barrier_t;
	 * 
	 * count is a non-saturating counter to distinguish events
	 * threads_left is the number of threads until the barrier releases
	 * threads_total is the number of threads to reset to
	 * 
	 */

	/* 
	 * BLAST barrier wait:
	 * 
	 * blast_barrier_wait(blast_barrier_t *barrier)
	 * 
	 * Read first word (count:threads_left) (memw_locked)
	 * If threads_left == 1, 
	 *  + increment count
	 *  + reset threads_left to threads_total
	 *  + write back word (memw_locked)
	 *  + wake threads_total
	 *  + return BLAST_BARRIER_SERIAL_THREAD
	 * else,
	 *  + decrement threads_left
	 *  + write back word (memw_locked)
	 *  + while re-read of count matches first read,
	 *    - wait
	 *  + return BLAST_BARRIER_OTHER
	 */

	.p2align 4
blast_barrier_wait:
	{
		allocframe(#8)
		r5 = memw(r0+#4)		// get reset value
	}
1:
	{
		r4 = #1
		r3.h = #1
	}
	r2 = memw_locked(r0)
	{
		p0 = vcmph.eq(r5:4,r3:2)	// is least-significant half 1?
		r1 = vaddh(r2,r3)		// add 1 to upper half, lower don't care
	}
	{
		if (!p0) jump 4f		// Nope, go to wait
		r1 = combine(r1.h,r5.l)		// incremented_count:threads_total
		r3 = vsubh(r2,r4)		// sub 1 to lower half, upper unchanged
	}
	memw_locked(r0,p0) = r1
	{
		if (!p0) jump 1b		// not atomic, start over
		r1 = r5
	}
	trap0(#BLAST_TRAP_FUTEX_WAKE)
	{
		deallocframe
	}
	{
		r0 = #BLAST_BARRIER_SERIAL_THREAD
		jumpr r31
	}
	.falign
4:
	// we will probably need to block, waiting for others
	memw_locked(r0,p0) = r3
	{
		if (!p0) jump 1b		// not atomic, start over
		if (p0) memd(r29+#0) = r25:24	// save callee-save regs
		if (p0) r25:24 = combine(r2,r0)	// orig count, ptr
		if (p0) r1 = r3			// expected val
	}
1:
	trap0(#BLAST_TRAP_FUTEX_WAIT)
	{
		r2 = memw(r24)			// read count:threads_left
		r3 = r25			// read orig_count:whatever
	}
	r3:2 = vlsrw(r3:2,#16)			// shift both right to get counts
	{
		p0 = cmp.eq(r3,r2)		// If they match,
		if (p0.new) jump:nt 1b		// spin again
	}
	{
		deallocframe			// read fp:lr, adj sp
		r25:24 = memd(r29+#0)		// read callee-save regs
	}
	{
		r0 = #BLAST_BARRIER_OTHER
		jumpr r31
	}


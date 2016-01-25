/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_plainmutex.S
 * 
 * This file contains the implementation for 
 * h2_plainmutex functionality 
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

	.text
	.global h2_plainmutex_unlock
	.global h2_plainmutex_trylock

FUNC_START h2_plainmutex_lock .text.h2.plainmutex .p2align 5
	{
		allocframe(#8)
	}
	{
		memd(r29+#0) = r25:24
		r24 = r0
		call h2_thread_myid
	}
2:
	{
		r25 = r0
		r1 = memw_locked(r24)
	}
	{
		p1 = cmp.eq(r1,#0)
		if (!p1.new) jump:nt 1f
		if (!p1.new) r0 = r24
	}
	{
		memw_locked(r24,p0) = r25
	}
	{
		if (!p0) jump 2b
		if (p0) r25:24 = memd(r29)
		if (p0) r31:30 = memd(r30)
		if (p0) r29 = add(r29,#16)
	}
	{
		r0 = #0
		jumpr r31
	}
	/* Lock nonzero, lock in kernel */
1:
	trap0(#H2_TRAP_FUTEX_LOCK_PI)
	{
		deallocframe
		r25:24 = memd(r29)
	}
	jumpr r31
FUNC_END h2_plainmutex_lock

/* EJP: ??? FUTEX_UNLOCK_PI probably checks W, and we don't actually write. Might be a problem for COW?. */
FUNC_START h2_plainmutex_unlock .text.h2.plainmutex .p2align 4
	allocframe(#8)
	{
		memd(r29+#0) = r25:24
		r24 = r0
		call h2_thread_myid
	}
	r25 = r0
2:
	{
		r1 = memw_locked(r24)
	}
	{
		p1 = cmp.eq(r1,r25)
		if (!p1.new) jump:nt 1f
		if (!p1.new) r0 = r24
		r2 = #0
	}
	{
		memw_locked(r24,p0) = r2
	}
	{
		if (!p0) jump 2b
		if (p0) r25:24 = memd(r29)
		if (p0) r31:30 = memd(r30)
		if (p0) r29 = add(r29,#16)
	}
	{
		r0 = #0
		jumpr r31
	}
1:
	trap0(#H2_TRAP_FUTEX_UNLOCK_PI)
	{
		p0 = cmp.gt(r0,#-1)
		if (!p0.new) jump:nt 2b
	}
	{
		dealloc_return
		r25:24 = memd(r29)
	}
FUNC_END h2_plainmutex_unlock

	/* 
	 * h2_plainmutex_trylock
	 * If mutex is 0, change it to TID and return 0
	 * Otherwise, return nonzero
	 */
FUNC_START h2_plainmutex_trylock .text.h2.plainmutex .falign
	allocframe(#8)
	{
		memd(r29+#0) = r25:24
		r24 = r0
		call h2_thread_myid
	}
2:
	{
		r2 = memw_locked(r24)
	}
	{
		p0 = cmp.eq(r2,#0)
		if (!p0.new) jump:nt 1f
		if (!p0.new) r0 = #-1
	}
	memw_locked(r24,p1) = r0
	{
		if (!p1) jump 2b
		if (p1) r0 = #0
	}
1:
	{
		deallocframe
		r25:24 = memd(r29)
	}
	jumpr r31
FUNC_END h2_plainmutex_trylock


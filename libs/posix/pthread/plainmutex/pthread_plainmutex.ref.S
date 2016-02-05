/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * pthread_plainmutex.S
 * 
 * This file contains the implementation for 
 * pthread_plainmutex functionality 
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START pthread_plainmutex_lock_id_canfail_np .text.pthread.plainmutex .p2align 5
#define LOCK r0
#define TID r1
#define TMP r2
	{
		TMP = memw_locked(LOCK)
	}
	{
		p0 = cmp.eq(TMP,#0)
		if (!p0.new) jump:nt 1f
	}
	{
		memw_locked(LOCK,p0) = TID
	}
	{
		if (p0) r0 = #0
		if (p0) jumpr r31
	}
	jump pthread_plainmutex_lock_id_canfail_np
1:
	/* Lock nonzero, lock in kernel */
	trap0(#H2_TRAP_FUTEX_LOCK_PI)
	jumpr r31
#undef LOCK
#undef TID
#undef TMP
FUNC_END pthread_plainmutex_lock_id_canfail_np

/* 
 * EJP: ??? FUTEX_UNLOCK_PI probably checks W, and we don't actually write. Might be a problem for COW?.
 * Possibly: FUTEX_UNLOCK_PI could generate the write fault on tlb hit but no w?
 */
FUNC_START pthread_plainmutex_unlock_id_canfail_np .text.pthread.plainmutex .p2align 4
#define LOCK r0
#define TID r1
#define TMP r2
#define ZERO r3
	{
		TMP = memw_locked(LOCK)
	}
	{
		p0 = cmp.eq(TMP,TID)
		if (!p0.new) jump:nt 1f
		ZERO = #0
	}
	{
		memw_locked(LOCK,p0) = ZERO
	}
	{
		if (p0) r0 = #0
		if (p0) jumpr r31
	}
	{
		jump pthread_plainmutex_unlock_id_canfail_np
	}
1:
	/* lock nonzero, lock in kernel */
	trap0(#H2_TRAP_FUTEX_UNLOCK_PI)
	jumpr r31
#undef LOCK
#undef TID
#undef TMP
#undef ZERO
FUNC_END pthread_plainmutex_unlock_id_canfail_np

	/* 
	 * pthread_plainmutex_trylock
	 * If mutex is 0, change it to TID and return 0
	 * Otherwise, return nonzero
	 */
FUNC_START pthread_plainmutex_trylock_id_np .text.pthread.plainmutex .falign
#define LOCK r0
#define TID r1
#define TMP r2
	{
		TMP = memw_locked(LOCK)
	}
	{
		p0 = cmp.eq(TMP,#0)
		if (!p0.new) jump:nt 1f
		if (!p0.new) r0 = #-1		/* EJP: FIXME: EBUSY */
	}
	memw_locked(LOCK,p0) = TID
	{
		if (!p0) jump pthread_plainmutex_trylock_id_np
		if (p0) r0 = #0
	}
1:
	jumpr r31
#undef LOCK
#undef TID
#undef TMP
FUNC_END pthread_plainmutex_trylock_id_np


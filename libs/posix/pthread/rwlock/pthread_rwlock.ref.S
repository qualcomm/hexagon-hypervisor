/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#if 0
/* Almost done, try later */
/*
 * pthread_tryrdlock(pthread_rwlock_t *lock)
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

	.text

#define RDLOCK_AMT 4
#define WRLOCK_VAL -16

FUNC_START pthread_tryrdlock .text.pthread.rwlock .p2align 5
#define LOCKPTR r0
#define LOCKVAL r1
#define LOCKVAL_NEW r2
	LOCKVAL = memw_locked(LOCKPTR)
	{
		LOCKVAL_NEW = add(LOCKVAL,#RDLOCK_AMT)
		p0 = cmp.gt(LOCKVAL,#-1)
		if (p0.new) jump:nt .Ltry_nope
	}
	memw_locked(LOCKPTR,p0) = LOCKVAL_NEW
	{
		if (p0) r0 = #0
		if (p0) jumpr r31
	}
	jump pthread_tryrdlock
.Ltry_nope:
	{
		r0 = #EBUSY
		jumpr r31
	}
#undef LOCKPTR
#undef LOCKVAL
#undef LOCKVAL_NEW
FUNC_END pthread_rwlock_tryrdlock

FUNC_START pthread_rwlock_trywrlock .text.pthread.rwlock .p2align 5
#define LOCKPTR r0
#define LOCKVAL r1
#define LOCKVAL_NEW r2
	LOCKVAL = memw_locked(LOCKPTR)
	{
		p0 = cmp.gtu(LOCKVAL,#(RDLOCK_AMT-1))
		if (p0.new) jump:nt .Ltry_nope
		LOCKVAL_NEW = or(LOCKVAL,#-16)
	}
	memw_locked(LOCKPTR,p0) = LOCKVAL_NEW
	{
		if (p0) r0 = #0
		if (p0) jumpr r31
	}
	jump pthread_trywrlock
#undef LOCKPTR
#undef LOCKVAL
#undef LOCKVAL_NEW
FUNC_END pthread_rwlock_trywrlock

FUNC_START pthread_rwlock_rdlock .text.pthread.rwlock .p2align 5
#define LOCKPTR r0
#define LOCKVAL r2
#define WAITERS r3
#define WAITERS_LOCKVAL r3:2
#define LOCKPTR_SAV r16
	WAITERS_LOCKVAL = memd_locked(LOCKPTR)
	{
		p0 = cmp.gt(LOCKVAL,#-1)
		if (!p0.new) jump:nt .Lrd_cantlock
		if (p0.new) LOCKVAL = add(LOCKVAL,#RDLOCK_AMT)
		if (!p0.new) WAITERS = add(WAITERS,#1)
	}
	memd_locked(LOCKPTR,p0) = WAITERS_LOCKVAL
	{
		if (p0) r0 = #0
		if (p0) jumpr r31
	}
	jump pthread_rdlock
.Lrd_cantlock:
	{
		memd_locked(LOCKPTR,p0) = WAITERS_LOCKVAL	// set waiters bit, increment waiters
		//p1 = cmp.gtu(LOCKVAL,#WRLOCK_VAL)
	}
	{
		if (!p0) jump pthread_rdlock
		//if (!p1) jump .Lrd_too_many
	}
	{
		allocframe(#8)
		memw(r29+#-16) = LOCKPTR_SAV
		LOCKPTR_SAV = LOCKPTR
	}
.Lrd_wait:
	r1:0 = combine(LOCKVAL,LOCKPTR_SAV)
	trap0(#H2_TRAP_FUTEX_WAIT)
.Lrd_try_after_wait:
	WAITERS_LOCKVAL = memd_locked(LOCKPTR_SAV)
	{
		p0 = cmp.gt(LOCKVAL,#-1)
		if (!p0.new) jump:nt .Lrd_wait
		if (p0.new) LOCKVAL = add(LOCKVAL,#RDLOCK_AMT)
		if (p0.new) WAITERS = add(WAITERS,#-1)
	}
	memd_locked(LOCKPTR_SAV,p0) = WAITERS_LOCKVAL
	{
		if (p0) LOCKPTR_SAV = memw(r29)
		if (p0) dealloc_return
		if (p0) r0 = #0
	}
	jump .Lrd_try_after_wait
#undef LOCKPTR
#undef LOCKVAL
#undef WAITERS
#undef WAITERS_LOCKVAL
#undef LOCKPTR_SAV
FUNC_END pthread_rwlock_rdlock

FUNC_START pthread_rwlock_wrlock .text.pthread.rwlock .p2align 5
#define LOCKPTR r0
#define LOCKVAL r2
#define WAITERS r3
#define WAITERS_LOCKVAL r3:2
#define LOCKPTR_SAV r16
	WAITERS_LOCKVAL = memd_locked(LOCKPTR)
	{
		p0 = cmp.eq(LOCKVAL,#0)
		if (p0.new) LOCKVAL = #WRLOCK_VAL
		if (!p0.new) WAITERS = add(WAITERS,#1)
		if (!p0.new) jump .Lwr_cantlock
	}
	memd_locked(LOCKPTR,p0) = WAITERS_LOCKVAL
	{
		if (p0) r0 = #0
		if (p0) jumpr r31
	}
	jump pthread_wrlock
.Lwr_cantlock:
	memd_locked(LOCKPTR,p0) = WAITERS_LOCKVAL
	if (p0) jump pthread_wrlock
	{
		allocframe(#8)
		memw(r29+#-16) = LOCKPTR_SAV
		LOCKPTR_SAV = LOCKPTR
	}
.Lwr_wait:
	r1:0 = combine(LOCKVAL,LOCKPTR_SAV)
	trap0(#H2_TRAP_FUTEX_WAIT)
.Lwr_try_after_wait:
	WAITERS_LOCKVAL = memd_locked(LOCKPTR_SAV)
	{
		p0 = cmp.eq(LOCKVAL,#0)
		if (!p0.new) jump:nt .Lwr_wait
		if (p0.new) LOCKVAL = #WRLOCK_VAL
		if (p0.new) WAITERS = add(WAITERS,#-1)
	}
	memd_locked(LOCKPTR_SAV,p0) = WAITERS_LOCKVAL
	{
		if (p0) LOCKPTR_SAV = memw(r29)
		if (p0) dealloc_return
		if (p0) r0 = #0
	}
	jump .Lwr_try_after_wait
#undef LOCKPTR
#undef LOCKVAL
#undef WAITERS
#undef WAITERS_LOCKVAL
#undef LOCKPTR_SAV
FUNC_END pthread_rwlock_wrlock

FUNC_START pthread_rwlock_unlock .text.pthread.rwlock .p2align 5
#define LOCKPTR r0
#define WAKEUP_NUM r1
#define WAKEUP_LOCKPTR r1:0
#define LOCKVAL r2
#define WAITERS r3
#define WAITERS_LOCKVAL r3:2
#define LOCKPTR_SAV r16
#define WAKEUP_SAV r17
#define WAKEUP_LOCKPTR_SAV r17:16
	{
		WAITERS_LOCKVAL = memd_locked(LOCKPTR,p0)
	}
	{
		p0 = cmp.eq(LOCKVAL,#WRLOCK_AMT)
		if (p0) LOCKVAL = #0
		if (!p0) LOCKVAL = add(LOCKVAL,#-RDLOCK_AMT)
	}
	{
		WAKEUP_NUM = mux(p0,WAITERS,#1)				// If we were a read lock, wake 1.  If a write lock, wake all
		memd(LOCKPTR,p2) = WAITERS_LOCKVAL
		p3 = cmp.eq(LOCKVAL,#0)					// we're totally freeing the lock
		p3 = cmp.gtu(WAITERS,#0)				// And there are waiters
	}
	{
		if (!p2) jump pthread_rwlock_unlock			// If not atomic, start all over.
	{
		if (!p3) r0 = #0					// If nothing to wake, return success
		if (!p3) jumpr r31
	}
	{
		allocframe(#8)
		memd(r29+#-16) = WAKEUP_LOCKPTR_SAV
		WAKEUP_LOCKPTR_SAV = combine(WAKEUP_NUM,LOCKPTR)
	}
.Lwakeup:
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		p0 = cmp.gt(r0,#-1)
		if (p0.new) WAKEUP_LOCKPTR_SAV = memd(r29+#0)
		if (p0.new) dealloc_return:t
	}
	{
		WAKEUP_LOCKPTR = WAKEUP_LOCKPTR_SAV
		WAITERS_LOCKVAL = memd(LOCKPTR_SAV)			// make accessible
		jump .Lwakeup
	}

#endif

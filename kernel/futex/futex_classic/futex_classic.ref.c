/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <futex.h>
#include <dosched.h>
#include <hw.h>
#include <runlist.h>
#include <readylist.h>
#include <ring.h>
#include <check_sanity.h>
#include <hexagon_protos.h>
#include <globals.h>
#include <atomic.h>
#include <safemem.h>
#include <lowprio.h>
#include <id.h>
#include <vmwork.h>

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me)
{
	u32_t readval;
	pa_t pa;
	BKL_LOCK();
	if ((me->vmstatus & H2K_VMSTATUS_VMWORK) && (me->vmstatus & H2K_VMSTATUS_IE)) {
		BKL_UNLOCK();
		H2K_vm_do_work(me);
		return -1;
	}
	if (!H2K_safemem_check_and_lock(lock,SAFEMEM_R,&pa,me)) {
		BKL_UNLOCK();
		return -1;
	}
	pa >>= 2;
	readval = *lock;
	H2K_safemem_unlock();
	if (readval != val) {
		/* Changed while we were trying to enqueue */
		BKL_UNLOCK();
		return -1;
	}
	me->futex_ptr = pa;
	H2K_runlist_remove(me);
	me->r0100 = 0;
	me->status = H2K_STATUS_BLOCKED;
	H2K_futex_hash_add_ring(&H2K_gp->futexhash[FUTEX_HASHVAL(pa)],me);
	H2K_dosched(me,me->hthread);
	/* Unreachable */
	return 0;
}

/* futex_resume 
 * Pick the first N matching elements out of the queue.
 */
s32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me)
{
	u32_t hashval;
	u32_t n_woken = 0;
	H2K_thread_context **ring;
	H2K_thread_context *pos;
	H2K_thread_context *tmp;
	pa_t pa;

	if (n_to_wake == 0) return 0;

	/* Need to do the read, but only because we need the PA */
	if (!H2K_safemem_check_and_lock(lock,SAFEMEM_R,&pa,me)) return -1;
	H2K_safemem_unlock();
	pa >>= 2;
	hashval = FUTEX_HASHVAL(pa);

	BKL_LOCK();

	ring = &H2K_gp->futexhash[hashval];
	tmp = pos = *ring;
	if (tmp == NULL) {
		BKL_UNLOCK();
		return 0;
	}
	do {
		/* FIXME: pass full PA when supported */
		if ((tmp = H2K_futex_hash_remove_one((u32_t)pa,ring,&pos)) != NULL) {
			n_woken++;
		} else {
			break;
		}
	} while (n_woken < n_to_wake);
	return (s32_t)H2K_check_sanity_unlock(n_woken);
}


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

static inline void H2K_futex_pi_raise(u32_t prio, H2K_id_t destid)
{
	u32_t hashval;
	H2K_thread_context *dest;
	dest = H2K_id_to_context(destid);
	if (dest == NULL) return;
	if (dest->prio <= prio) return;
	if (dest->status == H2K_STATUS_BLOCKED) {
		/* Need to move to correct priority.  
		 * Don't know what thread is blocked on... 
		 * Need to get get hashval from dest somehow
		 */ 
		dest->prio = prio;
		if (dest->next == dest) /* Only thing in the list */ return;
		hashval = FUTEX_HASHVAL(dest->futex_ptr);
		H2K_ring_remove(&H2K_gp->futexhash[hashval],dest);
		H2K_futex_hash_add_ring(&H2K_gp->futexhash[FUTEX_HASHVAL(dest->futex_ptr)],dest);
	} else if (dest->status == H2K_STATUS_READY) {
		H2K_ready_remove(dest);
		dest->prio = prio;
		H2K_ready_insert(dest);
	} else if (dest->status == H2K_STATUS_RUNNING) {
		/* 
		 * EJP: FIXME: 
		 * Now that runlist structure has changed, this adjustment should be easier? 
		 */
		H2K_runlist_remove(dest);
		dest->prio = prio;
		H2K_runlist_push(dest);
		if (H2K_gp->priomask & (1<<dest->hthread)) {
			H2K_raise_lowprio();
		}
		/* Need to update lowprio */
	} else if (dest->status == H2K_STATUS_INTBLOCKED) {
		/* Waiting on interrupt, but we want it to have high priority when it starts up */
		dest->prio = prio;
	} else {
		/* Dead?  Should we notify someone about that? */
	}
}

s32_t H2K_futex_lock_pi(u32_t *lock, H2K_thread_context *me)
{
	union {
		u32_t val;
		H2K_id_t dest;
		struct {
			u32_t low5bits:5;
			u32_t tid:27;
		};
	} x;
	pa_t pa;
	BKL_LOCK();
	if ((me->vmstatus & H2K_VMSTATUS_VMWORK) && (me->vmstatus & H2K_VMSTATUS_IE)) {
		BKL_UNLOCK();
		H2K_vm_do_work(me);
		return -1;
	}
	if (!H2K_safemem_check_and_lock(lock,SAFEMEM_RW,&pa,me)) {
		BKL_UNLOCK();
		return -1;
	}
	pa >>= 2;
	H2K_atomic_setbit(lock,0);
	x.val = *lock;
	if (x.val == 0x1) {
		/* Lock was completely freed */
		/* Mark me as owner, no waiters */
		x.dest = H2K_id_from_context(me);
		*lock = x.val;
		H2K_safemem_unlock();
		BKL_UNLOCK();
		return 0;
	} else {
		x.low5bits = 0;
		H2K_safemem_unlock();
	}
	H2K_futex_pi_raise(me->prio,x.dest);
	me->futex_ptr = pa;
	H2K_runlist_remove(me);
	me->r0100 = 0;
	me->status = H2K_STATUS_BLOCKED;
	H2K_futex_hash_add_ring(&H2K_gp->futexhash[FUTEX_HASHVAL(pa)],me);
	/* Optimization: set continuation to some kind of check thing */
	H2K_dosched(me,me->hthread);
	/* Unreachable */
	return 0;
}

s32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me)
{
	u32_t hashval;
	H2K_thread_context *ret;
	H2K_thread_context **ring;
	H2K_thread_context *pos;
	pa_t pa;
	/* Lock */
	BKL_LOCK();
	/* Get best thread */
	if (!H2K_safemem_check_and_lock(lock,SAFEMEM_RW,&pa,me)) {
		BKL_UNLOCK();
		return -1;
	}
	pa >>= 2;
	hashval = FUTEX_HASHVAL(pa);
	ring = &H2K_gp->futexhash[hashval];
	pos = *ring;
	ret = H2K_futex_hash_remove_one(pa,ring,&pos);
	if (ret == NULL) {
		/* TBD: Do this more carefully */
		/* TBD: check return value? */
		H2K_atomic_swap(lock,0);
	} else {
		H2K_atomic_swap(lock,H2K_id_from_context(ret).raw+1);
	}
	H2K_safemem_unlock();
	/* Restore my priority */
	if (me->prio != me->base_prio) {
		H2K_runlist_remove(me);
		me->prio = me->base_prio;
		H2K_runlist_push(me);
	}
	return H2K_check_sanity_unlock(0);
}


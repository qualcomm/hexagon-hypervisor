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
#include <q6protos.h>
#include <globals.h>
#include <atomic.h>
#include <safemem.h>
#include <lowprio.h>
#include <id.h>

/* 
 * EJP: hash table aligned to it's size, so we 
 * should be able to use tableidx on the product 
 */
#define HASHVAL(X) (Q6_R_extractu_RII((((unsigned int)(X)) * 2654435761UL),FUTEX_HASHBITS,32-FUTEX_HASHBITS))

/* Data Structure Interface Functions */

IN_SECTION(".text.core.futex") static void H2K_futex_hash_add(pa_t pa, H2K_thread_context *me)
{
	u32_t hashval = HASHVAL(pa);
	H2K_thread_context *tmp;
	if (H2K_gp->futexhash[hashval] == NULL) {
		/* empty */
		me->next = me->prev = me;
		H2K_gp->futexhash[hashval] = me;
	} else if (me->prio < H2K_gp->futexhash[hashval]->prio) {
		/* me is best prio */
		me->next = H2K_gp->futexhash[hashval];
		me->prev = me->next->prev;
		me->next->prev = me;
		me->prev->next = me;
		H2K_gp->futexhash[hashval] = me;
	} else {
		/* Not the best priority, search backwards through lower prios (if any) */
		tmp = H2K_gp->futexhash[hashval]->prev;
		while (me->prio < tmp->prio) tmp = tmp->prev;
		me->next = tmp->next;
		me->prev = tmp;
		me->next->prev = me;
		me->prev->next = me;
	}
}

/*
 * UNSAFE: We assume that *lock is suitable for tinkering with.
 * This needs to be done in a safe way eventually.  It could
 * be uncached or a bad address space or whatnot.
 */

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me)
{
	u32_t readval;
	pa_t pa;
	BKL_LOCK();
	if (!H2K_safemem_check_and_lock(lock,SAFEMEM_R,&pa,me)) {
		BKL_UNLOCK();
		return -1;
	}
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
	H2K_futex_hash_add(pa,me);
	H2K_dosched(me,me->hthread);
	/* Unreachable */
	return 0;
}

/* FIXME: Need to return next state (for multi wake) as well as removed thread (for pi) */

IN_SECTION(".text.core.futex")
static H2K_thread_context *H2K_futex_hash_remove_one(pa_t lock, H2K_thread_context **ring, H2K_thread_context **pos)
{
	H2K_thread_context *tmp;
	H2K_thread_context *cur;
	H2K_thread_context *start = *ring;
	cur = *pos;
	if (start == NULL) return NULL;
	while (1) {
		if (cur->futex_ptr == lock) {
			tmp = cur->next;
			H2K_ring_remove(ring,cur);
			H2K_ready_append(cur);
			*pos = tmp;
			return cur;
		} else {
			cur = cur->next;
			if (cur == *ring) {
				return NULL;
			} else continue;
		}
	};
	/* Unreachable */
	return NULL;
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
	hashval = HASHVAL(pa);

	BKL_LOCK();

	ring = &H2K_gp->futexhash[hashval];
	tmp = pos = *ring;
	if (tmp == NULL) {
		BKL_UNLOCK();
		return 0;
	}
	do {
		if ((tmp = H2K_futex_hash_remove_one(pa,ring,&pos)) != NULL) {
			n_woken++;
		} else {
			break;
		}
	} while (n_woken < n_to_wake);
	return H2K_check_sanity_unlock(n_woken);
}

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
		hashval = HASHVAL(dest->futex_ptr);
		H2K_ring_remove(&H2K_gp->futexhash[hashval],dest);
		H2K_futex_hash_add(dest->futex_ptr,dest);
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
	if (!H2K_safemem_check_and_lock(lock,SAFEMEM_RW,&pa,me)) {
		BKL_UNLOCK();
		return -1;
	}
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
	H2K_futex_hash_add(pa,me);
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
	hashval = HASHVAL(pa);
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

void H2K_futex_cancel(H2K_thread_context *dst)
{
}

void H2K_futex_init()
{
	int i;
	for (i = 0; i < FUTEX_HASHSIZE; i++) {
		H2K_gp->futexhash[i] = NULL;
	}
}


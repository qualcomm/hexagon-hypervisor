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

/* 
 * EJP: hash table aligned to it's size, so we 
 * should be able to use tableidx on the product 
 */
// H2K_thread_context *H2K_futexhash[FUTEX_HASHSIZE] __attribute__((aligned(FUTEX_HASHSIZE*4))) IN_SECTION(".data.sched.futex");
#define HASHVAL(X) (Q6_R_extractu_RII((((unsigned int)(X)) * 2654435761UL),FUTEX_HASHBITS,32-FUTEX_HASHBITS))

/*
 * UNSAFE: We assume that *lock is suitable for tinkering with.
 * This needs to be done in a safe way eventually.  It could
 * be uncached or a bad address space or whatnot.
 */

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me)
{
	u32_t hashval = HASHVAL(lock);
	H2K_thread_context *tmp;
	BKL_LOCK();
	if (*lock != val) {
		/* Changed while we were trying to enqueue */
		BKL_UNLOCK();
		return -1;
	}
	me->futex_ptr = lock;
	H2K_runlist_remove(me);
	me->r0100 = 0;
	me->status = H2K_STATUS_BLOCKED;
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
	H2K_dosched(me,me->hthread);
	return 0;
}

/* futex_resume 
 * Pick the first N matching elements out of the queue.
 */
u32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me)
{
	H2K_thread_context *tmp,*tmp2;
	u32_t hashval = HASHVAL(lock);
	u32_t n_woken = 0;
	if (n_to_wake == 0) return 0;

	BKL_LOCK();
	if (H2K_gp->futexhash[hashval] == NULL) {
		BKL_UNLOCK();
		return 0;
	}
	tmp = H2K_gp->futexhash[hashval];
	do {
		if (tmp->futex_ptr == lock) {
			tmp2 = tmp->next;
			H2K_ring_remove(&H2K_gp->futexhash[hashval],tmp);
			H2K_ready_append(tmp);
			n_woken++;
			tmp = tmp2;
			continue;
		} else {
			tmp = tmp->next;
			if (tmp == H2K_gp->futexhash[hashval]) break;
		}
	} while ((n_woken < n_to_wake) && H2K_gp->futexhash[hashval]);
	return H2K_check_sanity_unlock(n_woken);
}

void H2K_futex_init()
{
	int i;
	for (i = 0; i < FUTEX_HASHSIZE; i++) {
		H2K_gp->futexhash[i] = NULL;
	}
}


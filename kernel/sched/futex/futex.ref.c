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

H2K_thread_context *H2K_futexhash[FUTEX_HASHSIZE] __attribute__((aligned(FUTEX_HASHSIZE*4))) IN_SECTION(".data.sched.futex");
#define HASHVAL(X) ((((unsigned int)(X)) >> 3) & (FUTEX_HASHSIZE-1))

/*
 * UNSAFE: We assume that *lock is suitable for tinkering with.
 * This needs to be done in a safe way eventually.  It could
 * be uncached or a bad address space or whatnot.
 */

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me)
{
	u32_t hashval = HASHVAL(lock);
	BKL_LOCK();
	if (*lock != val) {
		/* Changed while we were trying to enqueue */
		BKL_UNLOCK();
		return -1;
	}
	me->futex_ptr = lock;
	//H2K_ring_remove_append(&running,&futexqueue,me);
	H2K_runlist_remove(me);
	H2K_ring_append(&H2K_futexhash[hashval],me);
	H2K_dosched(me,me->hthread);
	return 0;
}

/* futex_find
 * Find the next thread in haystack that has a matching lock ptr 
 */
static H2K_thread_context *futex_find(H2K_thread_context *haystack, H2K_thread_context *start, u32_t *lock)
{
	H2K_thread_context *tmp = start;
	if (haystack == NULL) return haystack;
	do {
		if (tmp->futex_ptr == lock) return tmp;
		tmp = tmp->next;
	} while (tmp != haystack);
	return NULL;
}

/* Futuex_resume 
 * If we are only going to wake one thread, pick the highest priority one 
 * Else, pick oldest ones from queue 
 */
u32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me)
{
	H2K_thread_context *tmp,*tmp2;
	u32_t hashval = HASHVAL(lock);
	u32_t n_woken = 0;
	u32_t highest_prio = MAX_PRIOS, prio;
	BKL_LOCK();
	if (H2K_futexhash[hashval] == NULL) {
		BKL_UNLOCK();
		return 0;
	}
	tmp = H2K_futexhash[hashval];
	if (n_to_wake == 1) {
		tmp2 = NULL;
		do {
			if (lock != tmp->futex_ptr) {
				tmp = tmp->next;
				continue;
			}
			prio = tmp->prio;
			if (highest_prio > prio) tmp2 = tmp;
			highest_prio = Q6_R_min_RR(highest_prio,prio);
			tmp = tmp->next;
		} while (H2K_futexhash[hashval] != tmp);
		if (tmp2) {
			n_woken = 1;
			prio = highest_prio;
			H2K_ring_remove(&H2K_futexhash[hashval],tmp2);
			H2K_ready_append(tmp2);
		}
	} else {
		tmp2 = H2K_futexhash[hashval];
		do {
			tmp = futex_find(H2K_futexhash[hashval],tmp2,lock);
			if (tmp == NULL) break;
			tmp2 = tmp->next;
			prio = tmp->prio;
			highest_prio = Q6_R_min_RR(highest_prio,prio);
			H2K_ring_remove(&H2K_futexhash[hashval],tmp);
			H2K_ready_append(tmp);
			if (++n_woken >= n_to_wake) break;
		} while (1);
	}
	if (n_woken) {
		return H2K_check_sanity_unlock(n_woken); 
	} else {
		BKL_UNLOCK();
		return n_woken;
	}
}

void H2K_futex_init()
{
	int i;
	for (i = 0; i < FUTEX_HASHSIZE; i++) {
		H2K_futexhash[i] = NULL;
	}
}


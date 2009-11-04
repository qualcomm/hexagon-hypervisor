/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>

BLASTK_thread_context *BLASTK_futexhash[FUTEX_HASHSIZE] __attribute__((aligned(FUTEX_HASHSIZE*4)));
#define HASHVAL(X) ((((unsigned int)(X)) >> 3) & (FUTEX_HASHSIZE-1))

/*
 * UNSAFE: We assume that *lock is suitable for tinkering with.
 * This needs to be done in a safe way eventually.  It could
 * be uncached or a bad address space or whatnot.
 */

int BLASTK_futex_wait(unsigned int *lock, unsigned int val, BLASTK_thread_context *me)
{
	unsigned int hashval = HASHVAL(lock);
	BKL_LOCK(&BLASTK_bkl);
	if (*lock != val) {
		/* Changed while we were trying to enqueue */
		BKL_UNLOCK(&BLASTK_bkl);
		return -1;
	}
	me->futex_ptr = lock;
	//BLASTK_ring_remove_append(&running,&futexqueue,me);
	runlist_remove(me);
	BLASTK_ring_append(&BLASTK_futexhash[hashval],me);
	BLASTK_dosched_with_lock(me,me->hthread);
	return 0;
}

/* futex_find
 * Find the next thread in haystack that has a matching lock ptr 
 */
static BLASTK_thread_context *futex_find(BLASTK_thread_context *haystack, BLASTK_thread_context *start, unsigned int *lock)
{
	BLASTK_thread_context *tmp = start;
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
int BLASTK_futex_resume(unsigned int *lock, unsigned int n_to_wake, BLASTK_thread_context *me)
{
	BLASTK_thread_context *tmp,*tmp2,*BLASTK_futexhash[hashval];
	unsigned int hashval = HASHVAL(lock);
	int n_woken = 0;
	int highest_prio = MAX_PRIOS, prio;
	BKL_LOCK(&BLASTK_bkl);
	if (BLASTK_futexhash[hashval] == NULL) {
		BKL_UNLOCK(&BLASTK_bkl);
		return 0;
	}
	tmp = BLASTK_futexhash[hashval];
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
		} while (BLASTK_futexhash[hashval] != tmp);
		if (tmp2) {
			n_woken = 1;
			prio = highest_prio;
			BLASTK_ring_remove(&BLASTK_futexhash[hashval],tmp2);
			ready_append(tmp2);
		}
	} else {
		tmp2 = BLASTK_futexhash[hashval];
		do {
			tmp = futex_find(BLASTK_futexhash[hashval],tmp2,lock);
			if (tmp == NULL) break;
			tmp2 = tmp->next;
			prio = tmp->prio;
			highest_prio = Q6_R_min_RR(highest_prio,prio);
			BLASTK_ring_remove(&BLASTK_futexhash[hashval],tmp);
			ready_append(tmp);
			if (++n_woken >= n_to_wake) break;
		} while (1);
	}
	if (n_woken) BLASTK_check_sanity(); 
	BKL_UNLOCK(&BLASTK_bkl);
	return n_woken;
}


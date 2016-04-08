/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <futex.h>
#include <c_std.h>
#include <ring.h>
#include <globals.h>
#include <id.h>
#include <readylist.h>

/* Data Structure Interface Functions */

void H2K_futex_hash_add_ring(H2K_thread_context **ring, H2K_thread_context *me)
{
	H2K_thread_context *tmp;
	if (*ring == NULL) {
		/* empty */
		me->next = me->prev = me;
		*ring = me;
	} else if (me->prio < (*ring)->prio) {
		/* me is best prio */
		me->next = *ring;
		me->prev = me->next->prev;
		me->next->prev = me;
		me->prev->next = me;
		*ring = me;
	} else {
		/* Not the best priority, search backwards through lower prios (if any) */
		tmp = (*ring)->prev;
		while (me->prio < tmp->prio) tmp = tmp->prev;
		me->next = tmp->next;
		me->prev = tmp;
		me->next->prev = me;
		me->prev->next = me;
	}
}

/* 
 * This returns the next position in the ring (for multi wake) 
 * as well as removed thread (for pi)
 * Thus we avoid re-iterating over the first elements in the ring
 */

H2K_thread_context *H2K_futex_hash_remove_one(u32_t locklo, H2K_thread_context **ring, H2K_thread_context **pos)
{
	H2K_thread_context *tmp;
	H2K_thread_context *cur;
	H2K_thread_context *start = *ring;
	cur = *pos;
	if (start == NULL) return NULL;
	while (1) {
		if (cur->futex_ptr_lo == locklo) {
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


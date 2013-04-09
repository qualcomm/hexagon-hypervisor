/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <physmem.h>
#include <buddy.h>
#include <ring.h>
#include <h2/h2_mutex.h>

h2_mutex_t buddy_mutex;

/* 
 * XXX: FIXME: We eventually want multiple pools.
 * This basically means that we have a default pool/freelist
 * and then secondary pools/freelists
 * The secondary pools are for things like TCM that we may want 
 * special allocation for.
 */
freebuddy_t *freelist[MAX_ORDER+1];

static inline void *buddy_merged_va(void *me, void *buddy)
{
	me = (void *)((u32_t)me & (u32_t)buddy);
	return me;
}

static void _buddy_free(void *me)
{
	ppage_t *me_ppage = va_to_ppage(me);
	int order = me_ppage->order;
	freebuddy_t *buddy = buddy_page(me,order);
	ppage_t *buddy_ppage = va_to_ppage(buddy);
	if ((order < MAX_ORDER) && (buddy_ppage->status == PHYS_PAGE_FREE) 
			&& (buddy_ppage->order == me_ppage->order)) {
		h2_ring_remove(&freelist[order],buddy);
		me_ppage->order = buddy_ppage->order = order+1;
		me = buddy_merged_va(me,buddy);
		_buddy_free(me);
	} else {
		h2_ring_insert(&freelist[order],me);
	}
}

/* Note: memory should already be in PHYS_PAGE_FREE status */
void buddy_free(void *me)
{
	h2_mutex_lock(&buddy_mutex);
	_buddy_free(me);
	h2_mutex_unlock(&buddy_mutex);
}

static void *_buddy_alloc(int order);

static void *buddy_split(int order)
{
	freebuddy_t *big;
	freebuddy_t *buddy;
	ppage_t *big_ppage;
	ppage_t *buddy_ppage;
	if ((big = _buddy_alloc(order+1)) == NULL) {
		return NULL;
	}
	buddy = buddy_page(big,order);
	big_ppage = va_to_ppage(big);
	buddy_ppage = va_to_ppage(buddy);
	big_ppage->order = buddy_ppage->order = order;
	// buddy->me_ppage = buddy_ppage;
	h2_ring_insert(&freelist[order],buddy);
	return big;
}

static void *_buddy_alloc(int order)
{
	void *ret;
	if (order > MAX_ORDER) return NULL;
	if (freelist[order] == NULL) {
		return buddy_split(order);
	} else {
		ret = freelist[order];
		h2_ring_remove(&freelist[order],ret);
		return ret;
	}
}

void *buddy_alloc(int order)
{
	void *ret;
	h2_mutex_lock(&buddy_mutex);
	ret = _buddy_alloc(order);
	h2_mutex_unlock(&buddy_mutex);
	return ret;
}

void buddy_init()
{
	int i;
	for (i = 0; i <= MAX_ORDER; i++) {
		freelist[i] = NULL;
	}
}


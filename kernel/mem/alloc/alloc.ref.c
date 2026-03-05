/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <alloc.h>
#include <max.h>
#include <spinlock.h>
#include <hw.h>

#define BYTES(X) ((X) * sizeof(H2K_mem_alloc_tag_t))
#define WORDS(X) (((X) + sizeof(H2K_mem_alloc_tag_t)) / sizeof(H2K_mem_alloc_tag_t))
#define UNITS(X) (((X) + ALLOC_UNIT) / ALLOC_UNIT)

H2K_mem_alloc_tag_t *H2K_mem_alloc_free_spinlocked(u32_t *ptr) {

	H2K_mem_alloc_tag_t *tag = (H2K_mem_alloc_tag_t *)ptr - 1;

	tag->released = 0;
	u32_t prev_size = (tag - 1)->size;
	H2K_mem_alloc_tag_t *prev = tag - prev_size;  // bogus if prev block in use
	H2K_mem_alloc_tag_t *next = (tag + tag->size);
	H2K_mem_alloc_tag_t *next_next = (next + next->size); // bogus for last block

	if (next->size != 0 && next_next->free) { // merge with next
		tag->size += next->size;
	}
	if (tag->free) { // merge with previous
		prev->size += tag->size;
		tag = prev;
	}
	(tag + tag->size - 1)->size = tag->size; // size at end of freed block
	(tag + tag->size)->free = 1;
	
	return tag;
}

/* Request mem, in bytes.  Return size and pointer to beginning of aligned space, or NULL */
H2K_mem_alloc_block_t H2K_mem_alloc_get(u32_t request) {

	H2K_mem_alloc_tag_t *tag = &(H2K_gp->alloc_heap)[ALLOC_UNIT - 1]; // first tag
	H2K_mem_alloc_tag_t *splinter;
	int request_units;
	int request_words;
	H2K_mem_alloc_block_t ret;

	ret.raw = 0;

	H2K_spinlock_lock(&(H2K_gp->alloc_heap_lock));
	while (1) {
		if (tag->released) {  // try to free
			BKL_LOCK();  // can't free until H2K_switch() is done with the block
			tag = H2K_mem_alloc_free_spinlocked((u32_t *)tag + 1);
			BKL_UNLOCK();
		}

		if (!(tag + tag->size)->free || BYTES(tag->size - 1) < request) {
			tag += tag->size;
			if (tag->size == 0) { // all out of bacon today
				H2K_spinlock_unlock(&(H2K_gp->alloc_heap_lock));
				return ret;
			}
		} else break;
	}

	/* Now we have a free block that's big enough */
	request_units = UNITS(WORDS(request));
	request_words = request_units * ALLOC_UNIT;

	splinter = tag + request_words;
	if (tag->size - request_words >= ALLOC_UNIT) { // ok to split
		splinter->size = tag->size - request_words;
		splinter->free = 0;  // previous block allocated
		splinter->released = 0;
		(tag + tag->size - 1)->size = splinter->size;  // size at end of splinter block
		/* free bit for splinter is already set */
		tag->size = request_words;
	} else { // just mark the block allocated
		(tag + tag->size)->free = 0;
	}

	H2K_spinlock_unlock(&(H2K_gp->alloc_heap_lock));
	ret.ptr = (u32_t *)(tag + 1);
	ret.size = (tag->size - 1) * sizeof(H2K_mem_alloc_tag_t);
	return ret;
}

H2K_mem_alloc_tag_t *H2K_mem_alloc_free(void *vptr) {
	u32_t *ptr = vptr;
	H2K_mem_alloc_tag_t *ret;
	H2K_spinlock_lock(&(H2K_gp->alloc_heap_lock));
	ret = H2K_mem_alloc_free_spinlocked(ptr);
	H2K_spinlock_unlock(&(H2K_gp->alloc_heap_lock));
	return ret;
}

/* Assume addr is cache-aligned */
void H2K_mem_alloc_init(H2K_mem_alloc_tag_t addr[], u32_t size) {
	
	H2K_gp->alloc_heap = addr;
	H2K_gp->alloc_heap_size = size;

	/* Last word before start of aligned space holds the tag for the first free
		 block, which contains all allocatable space */
	(H2K_gp->alloc_heap)[ALLOC_UNIT - 1].size = (H2K_gp->alloc_heap_size) - ALLOC_UNIT;
	(H2K_gp->alloc_heap)[ALLOC_UNIT - 1].free = 0;    // bogus previous is "allocated"
	(H2K_gp->alloc_heap)[ALLOC_UNIT - 1].released = 0;
	
	/* The last word in the heap holds the tag for the "next" block, which
		 doesn't exist, but we need it for the last block's free bit */
	(H2K_gp->alloc_heap)[(H2K_gp->alloc_heap_size) - 1].size = 0;     // mark end of allocation space
	(H2K_gp->alloc_heap)[(H2K_gp->alloc_heap_size) - 1].free = 1;     // the wilderness is wild and free
	(H2K_gp->alloc_heap)[(H2K_gp->alloc_heap_size) - 1].released = 0; // don't care

	H2K_spinlock_init(&(H2K_gp->alloc_heap_lock));

}


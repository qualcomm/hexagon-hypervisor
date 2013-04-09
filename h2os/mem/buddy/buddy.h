/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2OS_BUDDY_H
#define H2OS_BUDDY_H 1

#include <c_std.h>
#include <vm.h>

typedef struct freebuddy_struct {
	struct freebuddy_struct *next;
	struct freebuddy_struct *prev;
	//ppage_t *me_ppage;
} freebuddy_t;

static inline void *buddy_page(void *page, int order)
{
	u32_t page_raw = (u32_t)page;
	page_raw ^= (1<<PAGE_SHIFT)<<order;
	return (void *)page_raw;
}

void buddy_init();
void buddy_free(void *block_to_free);
void *buddy_alloc(int order);

#define MAX_ORDER 9

#ifdef DEBUG
#include <stdio.h>
#include <ring.h>
extern freebuddy_t *freelist[];
static inline void buddy_dump()
{
	int i;
	for (i = 0; i <= MAX_ORDER; i++) {
		printf("order %d: %p\n",i,freelist[i]);
		h2_ring_dump(&freelist[i]);
	}
}
#else

static inline void buddy_dump() {}
#endif

#endif

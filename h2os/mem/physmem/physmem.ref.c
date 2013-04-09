/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <physmem.h>
#include <buddy.h>

/* This uses <0.1% of memory @ 32-bit ppage_t */
/* Each 4KB page will hold ppage_t's for 4MB of memory. */
/* But for 512MB max memory, that's 512KB ppage_t's. */
/* FIXME: At initialization, we can free unused pages blocks for small memories */

ppage_t pages[MAX_MEM_SIZE >> PAGE_SHIFT] __attribute__((aligned(4096)));

void physmem_init()
{
	int i;
	ppage_t initval;
	initval.raw = 0;
	initval.status = PHYS_PAGE_INVALID;
	for (i = 0; i < MAX_MEM_SIZE >> PAGE_SHIFT; i++) {
		pages[i] = initval;
	}
}

void physmem_add_4k_page(void *addr)
{
	ppage_t *pp = va_to_ppage(addr);
	pp->status = PHYS_PAGE_FREE;
	buddy_free(addr);
}


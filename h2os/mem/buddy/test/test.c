/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#define DEBUG 1
#include <buddy.h>
#include <physmem.h>
#include <stdlib.h>
#include <ring.h>

#define PAGES 2048

void FAIL(const char *x)
{
	puts("FAIL");
	puts(x);
	exit(1);
}

int main()
{
	int i;
	void *mem;
	void *page;
	void *head;
	u32_t memidx;
	buddy_init();
	physmem_init();
	buddy_dump();
	mem = malloc(PAGES*4096);
	printf("malloc returned %p\n",mem);
	memidx = (u32_t)mem;
	memidx = memidx >> PAGE_SHIFT;
	for (i = 1; i < PAGES; i++) {
		page = (void *)((memidx+i)<<PAGE_SHIFT);
		physmem_add_4k_page(page);
	}
	buddy_dump();
	for (i = 1; i < PAGES; i++) {
		if ((buddy_alloc(0)) == NULL) FAIL("Couldn't realloc");
		//buddy_dump();
	}
	puts("TEST PASSED\n");
	return 0;
}


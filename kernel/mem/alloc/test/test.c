/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <alloc.h>
#include <max.h>

#include <stdio.h>
#include <stdlib.h>

#define UNIT_BYTES (ALLOC_UNIT * sizeof(H2K_mem_alloc_tag_t))

#define SIZE (ALLOC_UNIT * 4)

#define FAIL_SIZE(unit, expect, val) FAIL("Test %d Unit %u: Expected size %u, got %u\n", test, unit, expect, val)
#define FAIL_FREE(unit, expect, val) FAIL("Test %d Unit %u: Expected free %u, got %u\n", test, unit, expect, val)
#define FAIL_BSIZE(unit, expect, val) FAIL("Test %d Unit %u: Expected return size %u, got %u\n", test, unit, expect, val)
#define FAIL_BPTR(unit, expect, val) FAIL("Test %d Unit %u: Expected pointer 0x%08x, got 0x%08x\n",test , unit, expect, val)
#define FAIL_NULL(val) FAIL("Test %d: Expected NULL pointer, got 0x%08x\n", test, val)
#define FAIL_FSIZE(unit, expect, val) FAIL("Test %d Unit %u: Expected freed size %u, got %u\n", test, unit, expect, val)

#define FAIL(...) \
	puts("FAIL"); \
	printf(__VA_ARGS__); \
	exit(1);

H2K_mem_alloc_tag_t Heap[SIZE] __attribute__((aligned(ALLOC_UNIT))) = {{{.size = 0, .free = 0}}};

int test = 1;

/* void FAIL(const char *str, u32_t unit, u32_t expect, u32_t val) */
/* { */
/* 	puts("FAIL"); */
/* 	printf(str, unit, expect, val); */
/* 	exit(1); */
/* } */

void check_heap(u32_t unit, u32_t size, u32_t free) {

	H2K_mem_alloc_tag_t *tag = &Heap[unit * ALLOC_UNIT - 1];

	if (tag->size != size) {
		FAIL_SIZE(unit, size, tag->size);
	}

	if ((tag + tag->size)->free != free) {
		FAIL_FREE(unit, free, (tag + tag->size)->free);
	}

	printf("OK Test %d Unit %u:  size %u  %s\n", test, unit, size, (free ? "free" : "allocated"));
}

void check_block(u32_t unit, H2K_mem_alloc_block_t block) {

	H2K_mem_alloc_tag_t *tag = &Heap[unit * ALLOC_UNIT - 1];

	if (block.ptr != (u32_t *)tag + 1) {
		FAIL_BPTR(unit, (u32_t)tag + 1, (u32_t)block.ptr);
	}

	if (block.size != (tag->size - 1) * sizeof(H2K_mem_alloc_tag_t)) {
		FAIL_BSIZE(unit, (tag->size - 1) * sizeof(H2K_mem_alloc_tag_t), block.size);
	}

	printf("OK Test %d Unit %u:  pointer 0x%08x  size %u\n", test, unit, (u32_t)block.ptr, block.size);
	test++;
}

void check_null(H2K_mem_alloc_block_t block) {

	if (block.ptr != NULL) {
		FAIL_NULL((u32_t)block.ptr);
	}

	printf("OK Test %d:  NULL\n", test);
	test++;
}

void check_freed(u32_t unit, u32_t freed) {

	H2K_mem_alloc_tag_t *tag = &Heap[unit * ALLOC_UNIT - 1];

	if (freed != tag->size) {
		FAIL_FSIZE(unit, tag->size, freed);
	}

	printf("OK Test %d Unit %u:  freed size %u\n", test, unit, freed);
	test++;
}

int main() {

	H2K_mem_alloc_block_t block1, block2, block3;
	u32_t freed;

	H2K_mem_do_alloc_init(Heap, SIZE);

	/* request too big */
	block1 = H2K_mem_alloc_get(UNIT_BYTES * 4);
	check_heap(1, ALLOC_UNIT * 3, 1);
	check_null(block1);

	/* request 0, should get min block #1 and split*/
	block1 = H2K_mem_alloc_get(0);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT * 2, 1);
	check_block(1, block1);

	/* request 1, min block # 2*/
	block2 = H2K_mem_alloc_get(1);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 0);
	check_heap(3, ALLOC_UNIT, 1);
	check_block(2, block2);

	/* request maximal unit */
	block3 = H2K_mem_alloc_get(UNIT_BYTES - sizeof(H2K_mem_alloc_tag_t));
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 0);
	check_heap(3, ALLOC_UNIT, 0);
	check_block(3, block3);

	/* free middle block, should not merge */
	freed = H2K_mem_alloc_free(block2.ptr);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 1);
	check_heap(3, ALLOC_UNIT, 0);
	check_freed(2, freed);
	
	/* request maximal unit + 1, should fail */
	block2 = H2K_mem_alloc_get(UNIT_BYTES - sizeof(H2K_mem_alloc_tag_t) + 1);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 1);
	check_heap(3, ALLOC_UNIT, 0);
	check_null(block2);

	/* free 3rd block, should merge */
	freed = H2K_mem_alloc_free(block3.ptr);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT * 2, 1);
	check_freed(2, freed);

	/* now maximal + 1 should succeed */
	block2 = H2K_mem_alloc_get(UNIT_BYTES - sizeof(H2K_mem_alloc_tag_t) + 1);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT * 2, 0);
	check_block(2, block2);

	/* free 2nd big block */
	freed = H2K_mem_alloc_free(block2.ptr);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT * 2, 1);
	check_freed(2, freed);

	/* free first block, should merge */
	freed = H2K_mem_alloc_free(block1.ptr);
	check_heap(1, ALLOC_UNIT * 3, 1);
	check_freed(1, freed);

	/* allocate 1, 2, 3, then free 3, 1, 2, should merge */
	block1 = H2K_mem_alloc_get(0);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT * 2, 1);
	check_block(1, block1);

	block2 = H2K_mem_alloc_get(0);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 0);
	check_heap(3, ALLOC_UNIT, 1);
	check_block(2, block2);

	block3 = H2K_mem_alloc_get(0);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 0);
	check_heap(3, ALLOC_UNIT, 0);
	check_block(3, block3);

	freed = H2K_mem_alloc_free(block3.ptr);
	check_heap(1, ALLOC_UNIT, 0);
	check_heap(2, ALLOC_UNIT, 0);
	check_heap(3, ALLOC_UNIT, 1);
	check_freed(3, freed);
	
	freed = H2K_mem_alloc_free(block1.ptr);
	check_heap(1, ALLOC_UNIT, 1);
	check_heap(2, ALLOC_UNIT, 0);
	check_heap(3, ALLOC_UNIT, 1);
	check_freed(1, freed);

	freed = H2K_mem_alloc_free(block2.ptr);
	check_heap(1, ALLOC_UNIT * 3, 1);
	check_freed(1, freed);

	puts("TEST PASSED");
}

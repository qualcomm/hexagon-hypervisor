/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <alloc.h>
#include <max.h>

#include <stdio.h>
#include <stdlib.h>

#define SIZE (ALLOC_UNIT * 4)

H2K_mem_alloc_tag_t Heap[SIZE] __attribute__((aligned(ALLOC_UNIT)));

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void check(int index, int size, int free) {

}

int main() {

	H2K_mem_alloc_block_t block1, block2, block3;

	H2K_mem_do_alloc_init(Heap, SIZE);

	block1 = H2K_mem_alloc_get(1);
	printf("Heap %x  ptr %x  size %d\n", Heap, block1.ptr, block1.size);

	block2 = H2K_mem_alloc_get(1);
	printf("Heap %x  ptr %x  size %d\n", Heap, block2.ptr, block2.size);

	block3 = H2K_mem_alloc_get(1);
	printf("Heap %x  ptr %x  size %d\n", Heap, block3.ptr, block3.size);

	H2K_mem_alloc_free(block2.ptr);
	block1 = H2K_mem_alloc_get(29);
	printf("Heap %x  ptr %x  size %d\n", Heap, block1.ptr, block1.size);

	H2K_mem_alloc_free(block3.ptr);
	block1 = H2K_mem_alloc_get(29);
	printf("Heap %x  ptr %x  size %d\n", Heap, block1.ptr, block1.size);

	puts("TEST PASSED");
}

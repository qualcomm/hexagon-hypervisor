/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <spinlock.h>
#include <hexagon_standalone.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_spinlock_t lock;

volatile int x;

void thread_func(void *unused)
{
	H2K_spinlock_lock(&lock);
	while (1) x = 1;
}

/*
 * For each test:
 *   * Build up the tree
 *   * Verify the tree is OK
 *   * 10 times:
 *      * Remove the head, verify tree
 *      * Add the former head back into the tree, verify tree
 *   * Bisect the tree 
 *   * Destructive Iterate the tree, freeing nodes
 */
int main()
{
	int i;
	lock = 1;
	H2K_spinlock_init(&lock);
	if (lock != 0) FAIL("init");
	H2K_spinlock_lock(&lock);
	if (lock == 0) FAIL("lock");
	if (H2K_spinlock_trylock(&lock) == 0) FAIL("trylock");
	H2K_spinlock_unlock(&lock);
	if (lock != 0) FAIL("unlock");
	if (H2K_spinlock_trylock(&lock) != 0) FAIL("trylock");
	H2K_spinlock_unlock(&lock);
	x = 0;
	H2K_spinlock_lock(&lock);
	if (lock == 0) FAIL("lock");
	for (i = 0; i < 10000; i++) {
		asm volatile ( "nop" : : :"memory");
	}
	if (x != 0) FAIL("not mutually exclusive");
	puts("TEST PASSED");
	return 0;
}


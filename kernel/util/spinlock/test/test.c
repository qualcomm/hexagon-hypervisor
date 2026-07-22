/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <spinlock.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_spinlock_t lock;

volatile int x;

/* True when the ticket lock is held: next ticket != now serving. */
static inline int spinlock_is_locked(H2K_spinlock_t v)
{
	return (u16_t)(v >> 16) != (u16_t)v;
}

void thread_func(void *unused)
{
	H2K_spinlock_lock(&lock);
	while (1) x = 1;
}

int main()
{
	int i;
	lock = 1;
	H2K_spinlock_init(&lock);
	if (lock != 0) FAIL("init");
	H2K_spinlock_lock(&lock);
	if (!spinlock_is_locked(lock)) FAIL("lock");
	if (H2K_spinlock_trylock(&lock) == 0) FAIL("trylock");
	H2K_spinlock_unlock(&lock);
	if (spinlock_is_locked(lock)) FAIL("unlock");
	if (H2K_spinlock_trylock(&lock) != 0) FAIL("trylock2");
	H2K_spinlock_unlock(&lock);
	x = 0;
	H2K_spinlock_lock(&lock);
	if (!spinlock_is_locked(lock)) FAIL("lock2");
	for (i = 0; i < 10000; i++) {
		asm volatile ( "nop" : : :"memory");
	}
	if (x != 0) FAIL("not mutually exclusive");
	puts("TEST PASSED");
	return 0;
}


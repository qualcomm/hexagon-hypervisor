/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Recursive pthread_mutex behavior:
 *   - same thread can lock N times, depth tracked, requires N unlocks
 *   - second thread blocks until owner fully unwinds the recursion
 *   - both PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP and
 *     pthread_mutexattr_settype(RECURSIVE) yield equivalent mutexes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define DEPTH 5
#define SETTLE_SPINS (1024*1024)

static pthread_mutex_t mtx_init = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t mtx_attr;
static h2_sem_t b_started;
static volatile int sec_acquired;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *secondary(void *arg)
{
	pthread_mutex_t *m = arg;
	h2_sem_up(&b_started);
	pthread_mutex_lock(m);
	sec_acquired = 1;
	pthread_mutex_unlock(m);
	return NULL;
}

static void exercise(pthread_mutex_t *m)
{
	pthread_t b;
	int i;

	sec_acquired = 0;

	for (i = 0; i < DEPTH; i++)
		if (pthread_mutex_lock(m) != 0) FAIL("recursive lock");

	if (pthread_create(&b, NULL, secondary, m) != 0) FAIL("secondary create");
	h2_sem_down(&b_started);

	for (i = 0; i < DEPTH - 1; i++)
		if (pthread_mutex_unlock(m) != 0) FAIL("recursive unlock");

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");
	if (sec_acquired) FAIL("secondary acquired before final unlock");

	if (pthread_mutex_unlock(m) != 0) FAIL("final unlock");
	if (pthread_join(b, NULL) != 0) FAIL("join secondary");
	if (!sec_acquired) FAIL("secondary never acquired");
}

int main(void)
{
	pthread_mutexattr_t attr;

	h2_sem_init_val(&b_started, 0);
	h2_handle_errors(1);
	puts("Starting mutex_recursive");

	exercise(&mtx_init);

	pthread_mutexattr_init(&attr);
	if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
		FAIL("settype RECURSIVE");
	pthread_mutex_init(&mtx_attr, &attr);
	pthread_mutexattr_destroy(&attr);

	exercise(&mtx_attr);

	puts("TEST PASSED");
	return 0;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage: N-1 children blocked in pthread_barrier_wait at a
 * count=N barrier when main returns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define BARRIER_COUNT 4
#define N_WAITERS (BARRIER_COUNT - 1)
#define SETTLE_SPINS (1024*1024)

static pthread_barrier_t barrier;
static h2_sem_t about_to_wait;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *waiter(void *arg)
{
	(void)arg;
	h2_sem_up(&about_to_wait);
	pthread_barrier_wait(&barrier);
	FAIL("barrier_wait returned");
	return NULL;
}

int main(void)
{
	pthread_t t[N_WAITERS];
	int i;

	pthread_barrier_init(&barrier, NULL, BARRIER_COUNT);
	h2_sem_init_val(&about_to_wait, 0);
	h2_handle_errors(1);
	puts("Starting stuck_in_barrier");

	for (i = 0; i < N_WAITERS; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");

	for (i = 0; i < N_WAITERS; i++)
		h2_sem_down(&about_to_wait);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	puts("TEST PASSED");
	return 0;
}

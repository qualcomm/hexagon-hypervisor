/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage: children blocked in sem_timedwait on a 0-valued POSIX
 * semaphore with a deadline far in the future, when main returns.
 * Exercises the reaper's timer-cancel path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <semaphore.h>
#include <posix_time.h>

#define N_WAITERS 3
#define SETTLE_SPINS (1024*1024)
#define DEADLINE_SECONDS 600

static sem_t empty_sem;
static h2_sem_t about_to_wait;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *waiter(void *arg)
{
	struct timespec ts;
	(void)arg;
	ts.tv_sec = DEADLINE_SECONDS;
	ts.tv_nsec = 0;
	h2_sem_up(&about_to_wait);
	sem_timedwait(&empty_sem, &ts);
	FAIL("sem_timedwait returned");
	return NULL;
}

int main(void)
{
	pthread_t t[N_WAITERS];
	int i;

	sem_init(&empty_sem, 0, 0);
	h2_sem_init_val(&about_to_wait, 0);
	h2_handle_errors(1);
	puts("Starting stuck_in_sem_timedwait");

	for (i = 0; i < N_WAITERS; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");

	for (i = 0; i < N_WAITERS; i++)
		h2_sem_down(&about_to_wait);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	puts("TEST PASSED");
	return 0;
}

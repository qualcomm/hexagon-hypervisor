/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage on abnormal termination: same scenario as
 * stuck_in_cond_wait, but main calls exit(1) instead of returning 0.
 * Verifies the reaper still runs cleanly when termination is abnormal.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define N_WAITERS 4
#define SETTLE_SPINS (1024*1024)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
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
	pthread_mutex_lock(&mtx);
	h2_sem_up(&about_to_wait);
	pthread_cond_wait(&cv, &mtx);
	FAIL("cond_wait returned");
	return NULL;
}

int main(void)
{
	pthread_t t[N_WAITERS];
	int i;

	h2_sem_init_val(&about_to_wait, 0);
	h2_handle_errors(1);
	puts("Starting exit1_main_cond_wait");

	for (i = 0; i < N_WAITERS; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");

	for (i = 0; i < N_WAITERS; i++)
		h2_sem_down(&about_to_wait);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	puts("TEST PASSED");
	exit(1);
}

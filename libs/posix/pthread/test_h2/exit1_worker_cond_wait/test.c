/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage when exit(1) is called from a non-main thread.
 * Several waiter threads are blocked in pthread_cond_wait. Main blocks
 * on a sem so it is itself blocked. A separate "exiter" thread prints
 * TEST PASSED and calls exit(1).
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define N_WAITERS 3
#define SETTLE_SPINS (1024*1024)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static h2_sem_t about_to_wait;
static h2_sem_t main_park;	/* never posted */

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

static void *exiter(void *arg)
{
	int i;
	(void)arg;
	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");
	puts("TEST PASSED");
	exit(1);
	return NULL;
}

int main(void)
{
	pthread_t t[N_WAITERS], t_exit;
	int i;

	h2_sem_init_val(&about_to_wait, 0);
	h2_sem_init_val(&main_park, 0);
	h2_handle_errors(1);
	puts("Starting exit1_worker_cond_wait");

	for (i = 0; i < N_WAITERS; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");

	for (i = 0; i < N_WAITERS; i++)
		h2_sem_down(&about_to_wait);

	if (pthread_create(&t_exit, NULL, exiter, NULL) != 0) FAIL("exiter create");

	h2_sem_down(&main_park);
	FAIL("main unparked");
	return 0;
}

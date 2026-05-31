/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_cond_signal / pthread_cond_broadcast:
 *   - signal w/ 0 waiters is a no-op (and returns 0)
 *   - signal wakes exactly one waiter
 *   - broadcast wakes all N waiters
 *
 * pthread_cond_timedwait with deadline already past returns ETIMEDOUT
 * without blocking. (The current impl is a busy yield-loop -- see
 * stuck_in_cond_timedwait/test.c for context.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <posix_time.h>
#include <errno.h>

#define N_WAITERS 4

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static h2_sem_t about_to_wait;
static volatile int wake_count;

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
	wake_count++;
	pthread_mutex_unlock(&mtx);
	return NULL;
}

int main(void)
{
	pthread_t t[N_WAITERS];
	struct timespec ts;
	int i, r;

	h2_sem_init_val(&about_to_wait, 0);
	h2_handle_errors(1);
	puts("Starting cond_signal_broadcast");

	/* signal w/ no waiters: returns 0, no crash */
	if (pthread_cond_signal(&cv) != 0) FAIL("signal-zero failed");
	if (pthread_cond_broadcast(&cv) != 0) FAIL("broadcast-zero failed");

	/* signal wakes exactly one */
	wake_count = 0;
	for (i = 0; i < N_WAITERS; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");
	for (i = 0; i < N_WAITERS; i++) h2_sem_down(&about_to_wait);

	pthread_cond_signal(&cv);
	/* let exactly one wake; we synchronize by joining once below */
	/* spin until someone increments */
	while (wake_count == 0) asm volatile ("nop");
	if (wake_count != 1) FAIL("more than one waiter woke from signal");

	/* broadcast wakes the rest */
	pthread_cond_broadcast(&cv);
	for (i = 0; i < N_WAITERS; i++)
		if (pthread_join(t[i], NULL) != 0) FAIL("waiter join");
	if (wake_count != N_WAITERS) FAIL("broadcast did not wake all");

	/* timedwait with deadline already past -> ETIMEDOUT (no block) */
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	pthread_mutex_lock(&mtx);
	r = pthread_cond_timedwait(&cv, &mtx, &ts);
	pthread_mutex_unlock(&mtx);
	if (r != ETIMEDOUT) FAIL("past-deadline timedwait did not return ETIMEDOUT");

	puts("TEST PASSED");
	return 0;
}

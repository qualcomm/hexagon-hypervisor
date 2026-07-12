/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_mutex_unlock on an unowned mutex. Per
 * libs/posix/pthread/mutex/pthread_mutex.ref.c, the impl unconditionally
 * decrements depth and calls plainmutex_unlock when depth reaches 0.
 * Unlocking an already-unlocked mutex thus underflows depth, leaving the
 * plainmutex still released. This test asserts:
 *   - unlock-unowned returns 0 (impl does not error-check)
 *   - the mutex remains usable: a subsequent lock+unlock from another
 *     thread completes successfully and does not deadlock.
 *
 * The test uses two settle delays to detect a hang (sim ulimit safety net).
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static h2_sem_t worker_done;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *worker(void *arg)
{
	(void)arg;
	if (pthread_mutex_lock(&mtx) != 0) FAIL("worker lock");
	if (pthread_mutex_unlock(&mtx) != 0) FAIL("worker unlock");
	h2_sem_up(&worker_done);
	return NULL;
}

int main(void)
{
	pthread_t t;
	int r;

	h2_sem_init_val(&worker_done, 0);
	h2_handle_errors(1);
	puts("Starting neg_mutex_unlock_unowned");

	/* unlock without ever locking -- impl returns 0 (no error check) */
	r = pthread_mutex_unlock(&mtx);
	if (r != 0) FAIL("unlock-unowned did not return 0");

	/* mutex should still be usable from another thread */
	if (pthread_create(&t, NULL, worker, NULL) != 0) FAIL("worker create");
	h2_sem_down(&worker_done);
	if (pthread_join(t, NULL) != 0) FAIL("worker join");

	/* and from this thread */
	if (pthread_mutex_lock(&mtx) != 0) FAIL("main lock");
	if (pthread_mutex_unlock(&mtx) != 0) FAIL("main unlock");

	puts("TEST PASSED");
	return 0;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * POSIX semaphore corner cases:
 *   - sem_trywait on a 0-valued sem returns -1 (errno EAGAIN)
 *   - sem_post on a sem with multiple waiters wakes exactly one
 *   - sem_getvalue reflects the current count
 *   - sem_timedwait is currently aliased to sem_wait by the impl, so it
 *     does not honor the deadline -- a successful sem_post unblocks it.
 *
 * Note: the underlying pthread_sem_add_np returns a wakeup-count (or
 * EOVERFLOW), not the POSIX 0/-1, so sem_post return code is not strictly
 * checked here -- only that it does not error fatally.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <semaphore.h>
#include <posix_time.h>
#include <errno.h>

#define N_WAITERS 3

static sem_t s;
static volatile int wakes;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *waiter(void *arg)
{
	(void)arg;
	sem_wait(&s);
	__sync_fetch_and_add(&wakes, 1);
	return NULL;
}

int main(void)
{
	pthread_t t[N_WAITERS];
	int i, sval, r;
	struct timespec ts;

	h2_handle_errors(1);
	puts("Starting sem_corner");

	if (sem_init(&s, 0, 0) != 0) FAIL("sem_init");

	/* sem_trywait on 0-valued sem fails */
	r = sem_trywait(&s);
	if (r == 0) FAIL("trywait on empty succeeded");

	/* sem_post one wakes one of N */
	wakes = 0;
	for (i = 0; i < N_WAITERS; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");

	/* small delay for waiters to enter sem_wait */
	for (i = 0; i < (1<<20); i++) asm volatile ("nop");

	(void)sem_post(&s);
	while (wakes == 0) asm volatile ("nop");
	if (wakes != 1) FAIL("sem_post woke more than one");

	(void)sem_post(&s);
	(void)sem_post(&s);

	for (i = 0; i < N_WAITERS; i++)
		if (pthread_join(t[i], NULL) != 0) FAIL("waiter join");
	if (wakes != N_WAITERS) FAIL("not all waiters woke");

	/* sem_getvalue */
	sem_init(&s, 0, 7);
	if (sem_getvalue(&s, &sval) != 0) FAIL("sem_getvalue");
	if (sval != 7) FAIL("sem_getvalue wrong count");

	/* sem_timedwait on positive sem succeeds (impl is aliased to sem_wait) */
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	if (sem_timedwait(&s, &ts) != 0) FAIL("timedwait positive failed");

	puts("TEST PASSED");
	return 0;
}

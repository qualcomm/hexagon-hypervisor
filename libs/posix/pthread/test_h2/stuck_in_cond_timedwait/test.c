/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Originally intended as a "stuck in pthread_cond_timedwait" reaper test,
 * but the current impl (libs/posix/pthread/cond/pthread_cond_imp.ref.c)
 * implements pthread_cond_timedwait as a busy yield-loop -- it never
 * enters a blocking kernel syscall, so the reaper-precondition (every live
 * context blocked) cannot be met from this primitive alone.
 *
 * This test asserts that property: the worker eventually returns from
 * pthread_cond_timedwait with ETIMEDOUT (the busy-wait observes the
 * elapsed-nanos clock crossing the deadline). When the impl is upgraded
 * to a real timed futex wait, this test will need to be reworked.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <posix_time.h>
#include <errno.h>

#define DEADLINE_NANOS 1000000	/* 1 ms */

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static h2_sem_t worker_done;
static volatile int worker_ret;

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
	ts.tv_sec = 0;
	ts.tv_nsec = DEADLINE_NANOS;
	pthread_mutex_lock(&mtx);
	worker_ret = pthread_cond_timedwait(&cv, &mtx, &ts);
	pthread_mutex_unlock(&mtx);
	h2_sem_up(&worker_done);
	return NULL;
}

int main(void)
{
	pthread_t t;

	h2_sem_init_val(&worker_done, 0);
	h2_handle_errors(1);
	puts("Starting stuck_in_cond_timedwait (expected: timedwait is busy-wait)");

	if (pthread_create(&t, NULL, waiter, NULL) != 0) FAIL("waiter create");
	h2_sem_down(&worker_done);

	if (worker_ret != ETIMEDOUT)
		FAIL("expected ETIMEDOUT from busy-wait timedwait");

	puts("TEST PASSED");
	return 0;
}

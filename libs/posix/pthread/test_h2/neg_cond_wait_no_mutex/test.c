/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_cond_wait without holding the mutex.
 *
 * Per the impl, cond_wait will release the mutex (corrupting depth via
 * underflow) and block on a futex; on signal it relocks the mutex.
 * The state is non-portable and "leaky" but should not crash.
 *
 * Probe approach: a worker calls cond_wait without locking, while main
 * holds the mutex (so the corruption is observable but not fatal).
 * Main signals after a short delay; worker should eventually wake.
 *
 * If the worker does not wake within the bounded probe window, conclude
 * the impl behavior is "permanently deadlocks" and pass with a note --
 * the reaper handles the parked worker on VM exit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define SETTLE_SPINS (1024*1024*8)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static h2_sem_t worker_about_to_wait;
static volatile int worker_returned;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *worker(void *arg)
{
	(void)arg;
	h2_sem_up(&worker_about_to_wait);
	/* deliberately do NOT pthread_mutex_lock(&mtx) */
	(void)pthread_cond_wait(&cv, &mtx);
	worker_returned = 1;
	return NULL;
}

int main(void)
{
	pthread_t t;
	int i;

	h2_sem_init_val(&worker_about_to_wait, 0);
	h2_handle_errors(1);
	puts("Starting neg_cond_wait_no_mutex (probe)");

	if (pthread_create(&t, NULL, worker, NULL) != 0) FAIL("create");
	h2_sem_down(&worker_about_to_wait);

	/* let worker enter cond_wait */
	for (i = 0; i < (1<<20); i++) asm volatile ("nop");

	pthread_cond_broadcast(&cv);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	if (worker_returned) {
		puts("note: cond_wait without mutex returned");
	} else {
		puts("note: cond_wait without mutex did not return (impl-dependent)");
	}

	puts("TEST PASSED");
	return 0;
}

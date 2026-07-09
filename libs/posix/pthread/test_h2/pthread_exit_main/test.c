/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_exit-from-main coverage: main creates a mix of joinable and
 * detached worker threads, releases them, then calls pthread_exit instead
 * of returning. Per POSIX, that terminates only the main thread; the
 * process must remain alive so the workers continue to run.
 *
 * Each released worker spins long enough to give main time to fully
 * complete pthread_exit, then atomically increments a counter. The last
 * worker to increment prints TEST PASSED and terminates the process via
 * exit(0). If pthread_exit on the main thread incorrectly tore down the
 * whole process, no worker would print TEST PASSED and the booter would
 * see a non-zero exit status.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define N_JOINABLE	3
#define N_DETACHED	3
#define N_TOTAL		(N_JOINABLE + N_DETACHED)
#define SETTLE_SPINS	(1024*1024)

static volatile int counter;
static h2_sem_t go;
static h2_sem_t started;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *worker(void *arg)
{
	int my_idx;
	int i;

	(void)arg;
	h2_sem_up(&started);
	h2_sem_down(&go);
	/* Spin so main has time to enter and complete pthread_exit. */
	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");
	my_idx = __atomic_add_fetch(&counter, 1, __ATOMIC_SEQ_CST);
	if (my_idx == N_TOTAL) {
		puts("TEST PASSED");
		exit(0);
	}
	return NULL;
}

int main(void)
{
	pthread_t j[N_JOINABLE];
	pthread_t d[N_DETACHED];
	pthread_attr_t det_attr;
	int i;

	h2_sem_init_val(&go, 0);
	h2_sem_init_val(&started, 0);
	h2_handle_errors(1);
	puts("Starting pthread_exit_main");

	for (i = 0; i < N_JOINABLE; i++)
		if (pthread_create(&j[i], NULL, worker, NULL) != 0) FAIL("joinable create");

	if (pthread_attr_init(&det_attr) != 0) FAIL("attr init");
	if (pthread_attr_setdetachstate(&det_attr, PTHREAD_CREATE_DETACHED) != 0) FAIL("attr setdetached");
	for (i = 0; i < N_DETACHED; i++)
		if (pthread_create(&d[i], &det_attr, worker, NULL) != 0) FAIL("detached create");
	pthread_attr_destroy(&det_attr);

	/* Confirm every worker is parked on `go` before main exits. */
	for (i = 0; i < N_TOTAL; i++) h2_sem_down(&started);

	/* Release the workers; they will spin then race on the counter. */
	for (i = 0; i < N_TOTAL; i++) h2_sem_up(&go);

	/*
	 * Leave via pthread_exit instead of returning. Per POSIX this tears
	 * down only the main thread; the workers we just released must keep
	 * running and one of them will print TEST PASSED.
	 */
	pthread_exit(NULL);
}

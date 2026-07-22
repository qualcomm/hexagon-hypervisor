/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage: a joinable child completes its start_routine and enters
 * pthread_exit. After posting `waiters`, pthread_exit blocks on the
 * `joined` sem -- and no thread will ever call pthread_join. Main returns
 * with the child parked inside pthread_exit.
 *
 * Mentioned explicitly in commit 22fcca23.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define SETTLE_SPINS (1024*1024)

static h2_sem_t about_to_exit;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *worker(void *arg)
{
	(void)arg;
	h2_sem_up(&about_to_exit);
	/*
	 * Returning here invokes pthread_exit via pthread_trampoline.
	 * pthread_exit posts `waiters` and then blocks on `joined`,
	 * waiting for a pthread_join that will never come.
	 */
	return NULL;
}

int main(void)
{
	pthread_t t;
	int i;

	h2_sem_init_val(&about_to_exit, 0);
	h2_handle_errors(1);
	puts("Starting stuck_in_pthread_exit_joined");

	if (pthread_create(&t, NULL, worker, NULL) != 0) FAIL("worker create");
	h2_sem_down(&about_to_exit);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	puts("TEST PASSED");
	return 0;
}

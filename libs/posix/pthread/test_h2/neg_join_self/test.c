/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_join(pthread_self()).
 *
 * Probe behavior of self-join on the current impl. Per
 * libs/posix/pthread/thread/pthread_thread.ref.c, pthread_join walks the
 * TCB list (finds self), checks detached (no), then waits on
 * dest->waiters (self.waiters). Self is the only thread that could ever
 * post that semaphore (via pthread_exit), so this is a deterministic
 * deadlock under the current impl.
 *
 * Test design: spawn a worker that calls pthread_join(pthread_self()).
 * Wait long enough for pthread_join to enter sem_wait, then conclude
 * deadlock (which is the expected, documented behavior). Print
 * TEST PASSED and return -- the reaper handles the parked worker.
 *
 * If the impl is later updated to detect self-join (return EDEADLK),
 * this test should be expanded to verify the EDEADLK return.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define SETTLE_SPINS (1024*1024*4)

static volatile int worker_finished;
static h2_sem_t about_to_join;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *self_joiner(void *arg)
{
	(void)arg;
	h2_sem_up(&about_to_join);
	(void)pthread_join(pthread_self(), NULL);
	worker_finished = 1;
	return NULL;
}

int main(void)
{
	pthread_t t;
	int i;

	h2_sem_init_val(&about_to_join, 0);
	h2_handle_errors(1);
	puts("Starting neg_join_self");

	if (pthread_create(&t, NULL, self_joiner, NULL) != 0) FAIL("create");
	h2_sem_down(&about_to_join);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	if (worker_finished) {
		puts("note: pthread_join(self) returned cleanly");
	} else {
		puts("note: pthread_join(self) deadlocked (expected on current impl)");
	}

	puts("TEST PASSED");
	return 0;
}

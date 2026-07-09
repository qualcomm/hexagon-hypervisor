/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage: a *detached* worker thread calls exit(1) while a
 * joinable sibling is stuck in pthread_mutex_lock. Detached threads have
 * no joiner so the reaper must clean them up, and the abnormal exit
 * happens from a thread context that isn't main.
 *
 * The race is between the detached exiter calling exit(1) (whole-VM
 * teardown) and the blocker committing to a BLOCKED state inside
 * pthread_mutex_lock on a vmblock that is already being torn down.
 *
 * A signed spin count is a RUNTIME parameter (argv[1]); its SIGN selects
 * which racing thread absorbs the delay, sliding one side's timing across
 * the other's teardown so the reproducer can land inside the race window:
 *   argv[1] >= 0 : delay the EXITER by |argv[1]| spins (blocker races in
 *                  immediately) -- this is the historical default.
 *   argv[1] <  0 : delay the BLOCKER by |argv[1]| spins (exiter tears the
 *                  VM down immediately) so the blocker commits to the
 *                  mutex wait late, inside the teardown window.
 * It falls back to a compile-time default when no argument is supplied, so
 * the test also runs under the plain harness invocation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static h2_sem_t holder_in;
static h2_sem_t holder_park;	/* never posted */
static h2_sem_t blocker_attempting;
static h2_sem_t main_park;	/* never posted */

/* Spin counts split by sign of argv[1]; only one side is delayed at a time.
 * Default is 0 spins on both sides -- no artificial delay. */
static volatile int exiter_spins = 0;
static volatile int blocker_spins = 0;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *holder(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&mtx);
	h2_sem_up(&holder_in);
	h2_sem_down(&holder_park);
	FAIL("holder unparked");
	return NULL;
}

static void *blocker(void *arg)
{
	int i;
	(void)arg;
	h2_sem_up(&blocker_attempting);
	for (i = 0; i < blocker_spins; i++) asm volatile ("nop");
	pthread_mutex_lock(&mtx);
	FAIL("blocker acquired mutex");
	return NULL;
}

static void *detached_exiter(void *arg)
{
	int i;
	(void)arg;
	for (i = 0; i < exiter_spins; i++) asm volatile ("nop");
	puts("TEST PASSED");
	exit(1);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t t_h, t_b, t_e;

	if (argc > 1) {
		long d = strtol(argv[1], NULL, 0);
		if (d >= 0) {
			exiter_spins = (int)d;
			blocker_spins = 0;
		} else {
			blocker_spins = (int)(-d);
			exiter_spins = 0;
		}
	}

	h2_sem_init_val(&holder_in, 0);
	h2_sem_init_val(&holder_park, 0);
	h2_sem_init_val(&blocker_attempting, 0);
	h2_sem_init_val(&main_park, 0);
	h2_handle_errors(1);
	printf("Starting exit1_detached_worker_with_stuck exiter_spins=%d blocker_spins=%d\n",
	       exiter_spins, blocker_spins);

	if (pthread_create(&t_h, NULL, holder, NULL) != 0) FAIL("holder create");
	h2_sem_down(&holder_in);

	if (pthread_create(&t_b, NULL, blocker, NULL) != 0) FAIL("blocker create");
	h2_sem_down(&blocker_attempting);

	if (pthread_create(&t_e, NULL, detached_exiter, NULL) != 0) FAIL("exiter create");
	if (pthread_detach(t_e) != 0) FAIL("detach exiter");

	h2_sem_down(&main_park);
	FAIL("main unparked");
	return 0;
}

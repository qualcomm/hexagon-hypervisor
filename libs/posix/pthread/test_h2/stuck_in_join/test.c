/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage: child A is blocked in pthread_join(B); child B is itself
 * parked on a never-posted sem. Both are blocked when main returns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define SETTLE_SPINS (1024*1024)

static h2_sem_t b_ready;
static h2_sem_t b_park;		/* never posted */
static h2_sem_t a_about_to_join;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *target_b(void *arg)
{
	(void)arg;
	h2_sem_up(&b_ready);
	h2_sem_down(&b_park);
	FAIL("b unparked");
	return NULL;
}

static void *joiner_a(void *arg)
{
	pthread_t b = (pthread_t)(unsigned long)arg;
	h2_sem_up(&a_about_to_join);
	pthread_join(b, NULL);
	FAIL("a's pthread_join returned");
	return NULL;
}

int main(void)
{
	pthread_t tb, ta;
	int i;

	h2_sem_init_val(&b_ready, 0);
	h2_sem_init_val(&b_park, 0);
	h2_sem_init_val(&a_about_to_join, 0);
	h2_handle_errors(1);
	puts("Starting stuck_in_join");

	if (pthread_create(&tb, NULL, target_b, NULL) != 0) FAIL("b create");
	h2_sem_down(&b_ready);

	if (pthread_create(&ta, NULL, joiner_a, (void *)(unsigned long)tb) != 0)
		FAIL("a create");
	h2_sem_down(&a_about_to_join);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	puts("TEST PASSED");
	return 0;
}

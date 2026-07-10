/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Reaper coverage on abnormal termination: same scenario as
 * stuck_in_mutex, but main calls exit(1) instead of returning 0.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define SETTLE_SPINS (1024*1024)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static h2_sem_t a_holding;
static h2_sem_t b_attempting;
static h2_sem_t holder_park;

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
	h2_sem_up(&a_holding);
	h2_sem_down(&holder_park);
	FAIL("holder unparked");
	return NULL;
}

static void *blocker(void *arg)
{
	(void)arg;
	h2_sem_down(&a_holding);
	h2_sem_up(&b_attempting);
	pthread_mutex_lock(&mtx);
	FAIL("blocker should never acquire mutex");
	return NULL;
}

int main(void)
{
	pthread_t t_a, t_b;
	int i;

	h2_sem_init_val(&a_holding, 0);
	h2_sem_init_val(&b_attempting, 0);
	h2_sem_init_val(&holder_park, 0);
	h2_handle_errors(1);
	puts("Starting exit1_main_mutex");

	if (pthread_create(&t_a, NULL, holder, NULL) != 0) FAIL("holder create");
	h2_sem_down(&a_holding);
	h2_sem_up(&a_holding);

	if (pthread_create(&t_b, NULL, blocker, NULL) != 0) FAIL("blocker create");
	h2_sem_down(&b_attempting);

	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");

	puts("TEST PASSED");
	exit(1);
}

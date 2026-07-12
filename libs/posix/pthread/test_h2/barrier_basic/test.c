/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_barrier basics:
 *   - all N threads release; exactly one returns PTHREAD_BARRIER_SERIAL_THREAD
 *   - the barrier can be reused across multiple rounds
 *   - count=1 barrier returns immediately
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define BARRIER_COUNT 4
#define ROUNDS 3

static pthread_barrier_t barrier;
static atomic_u32_t serial_observed[ROUNDS];
static atomic_u32_t arrival_count[ROUNDS];

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *waiter(void *arg)
{
	int round, ret;
	(void)arg;
	for (round = 0; round < ROUNDS; round++) {
		ret = pthread_barrier_wait(&barrier);
		h2_atomic_add32(&arrival_count[round], 1);
		if (ret == PTHREAD_BARRIER_SERIAL_THREAD)
			h2_atomic_add32(&serial_observed[round], 1);
		else if (ret != 0)
			FAIL("barrier_wait returned unexpected value");
	}
	return NULL;
}

int main(void)
{
	pthread_t t[BARRIER_COUNT];
	pthread_barrier_t single;
	int i, r;

	for (i = 0; i < ROUNDS; i++) {
		h2_atomic_swap32(&serial_observed[i], 0);
		h2_atomic_swap32(&arrival_count[i], 0);
	}

	pthread_barrier_init(&barrier, NULL, BARRIER_COUNT);
	h2_handle_errors(1);
	puts("Starting barrier_basic");

	for (i = 0; i < BARRIER_COUNT; i++)
		if (pthread_create(&t[i], NULL, waiter, NULL) != 0) FAIL("waiter create");

	for (i = 0; i < BARRIER_COUNT; i++)
		if (pthread_join(t[i], NULL) != 0) FAIL("waiter join");

	for (i = 0; i < ROUNDS; i++) {
		if (h2_atomic_sub32(&arrival_count[i], 0) != BARRIER_COUNT)
			FAIL("not all threads arrived in a round");
		if (h2_atomic_sub32(&serial_observed[i], 0) != 1)
			FAIL("expected exactly one SERIAL_THREAD per round");
	}

	/* count=1: barrier_wait should return immediately */
	pthread_barrier_init(&single, NULL, 1);
	r = pthread_barrier_wait(&single);
	if (r != PTHREAD_BARRIER_SERIAL_THREAD && r != 0)
		FAIL("count=1 barrier returned unexpected value");

	puts("TEST PASSED");
	return 0;
}

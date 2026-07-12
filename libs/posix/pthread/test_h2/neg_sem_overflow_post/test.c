/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Semaphore overflow / saturation behavior. The h2 sem value is bounded
 * by PTHREAD_SEM_VALUE_MAX_NP. We post a large number of times (greater
 * than the max) and then verify that:
 *   - we can drain the sem the expected number of times without blocking
 *   - the count saturates / does not silently wrap to zero
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <semaphore.h>
#include <pthread_sem.h>

#define POSTS (PTHREAD_SEM_VALUE_MAX_NP + 100)

static sem_t s;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

int main(void)
{
	int sval;
	int i;
	int drained;

	h2_handle_errors(1);
	puts("Starting neg_sem_overflow_post");

	if (sem_init(&s, 0, 0) != 0) FAIL("sem_init");

	for (i = 0; i < POSTS; i++) (void)sem_post(&s);

	if (sem_getvalue(&s, &sval) != 0) FAIL("sem_getvalue");
	if (sval == 0) FAIL("count silently wrapped to zero");
	if (sval > PTHREAD_SEM_VALUE_MAX_NP) FAIL("count exceeded max");

	/* drain sem; must succeed exactly sval times via trywait */
	drained = 0;
	while (sem_trywait(&s) == 0) {
		drained++;
		if (drained > PTHREAD_SEM_VALUE_MAX_NP + 1) FAIL("drained too many");
	}
	if (drained == 0) FAIL("could not drain at all");

	puts("TEST PASSED");
	return 0;
}

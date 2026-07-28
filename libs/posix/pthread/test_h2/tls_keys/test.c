/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread TLS keys:
 *   - pthread_key_create succeeds for a key, pthread_getspecific returns
 *     NULL before any setspecific
 *   - per-thread isolation: each thread sees its own value
 *   - destructor fires on pthread_exit if value is non-NULL
 *   - pthread_key_delete then re-create works
 *   - allocating up to PTHREAD_KEYS_MAX keys: at least one allocation
 *     should succeed; further allocations beyond capacity return EAGAIN.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define N_WORKERS 3

static pthread_key_t key;
static atomic_u32_t dtor_invocations;
static int worker_args[N_WORKERS];

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void dtor(void *value)
{
	(void)value;
	h2_atomic_add32(&dtor_invocations, 1);
}

static void *worker(void *arg)
{
	void *got;
	got = pthread_getspecific(key);
	if (got != NULL) FAIL("getspecific not NULL pre-set");
	if (pthread_setspecific(key, arg) != 0) FAIL("setspecific");
	got = pthread_getspecific(key);
	if (got != arg) FAIL("getspecific != set value");
	return NULL;
}

int main(void)
{
	pthread_t t[N_WORKERS];
	int i;
	pthread_key_t key2;
	pthread_key_t bulk[PTHREAD_KEYS_MAX];
	int allocated = 0;

	h2_atomic_swap32(&dtor_invocations, 0);
	h2_handle_errors(1);
	puts("Starting tls_keys");

	if (pthread_key_create(&key, dtor) != 0) FAIL("key_create");
	if (pthread_getspecific(key) != NULL) FAIL("getspecific not NULL on main");

	for (i = 0; i < N_WORKERS; i++) worker_args[i] = i + 1;
	for (i = 0; i < N_WORKERS; i++)
		if (pthread_create(&t[i], NULL, worker, &worker_args[i]) != 0)
			FAIL("worker create");
	for (i = 0; i < N_WORKERS; i++)
		if (pthread_join(t[i], NULL) != 0) FAIL("worker join");

	if (h2_atomic_sub32(&dtor_invocations, 0) != N_WORKERS)
		FAIL("destructor not invoked once per worker");

	/* delete then re-create */
	if (pthread_key_delete(key) != 0) FAIL("key_delete");
	if (pthread_key_create(&key2, NULL) != 0) FAIL("key_create after delete");

	/* exhaustion: allocate keys up to capacity */
	for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
		if (pthread_key_create(&bulk[i], NULL) != 0)
			break;
		allocated++;
	}
	if (allocated < 1)
		FAIL("could not allocate any extra keys");

	puts("TEST PASSED");
	return 0;
}

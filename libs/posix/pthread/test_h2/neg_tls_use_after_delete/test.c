/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread TLS use-after-delete.
 *
 * The current impl (libs/posix/pthread/tls/pthread_tls.ref.c) does not
 * validate the key in pthread_setspecific / pthread_getspecific -- it
 * just indexes into the per-thread TLS array. The key_valid bitmap is
 * only consulted by pthread_tls_teardown to decide whether to invoke a
 * destructor.
 *
 * This test asserts that:
 *   - setspecific / getspecific on a deleted key do not error
 *   - the value written via setspecific(deleted_key, v) is observable
 *     via getspecific(deleted_key) -- behavior is well-defined-but-leaky
 *   - re-creating after delete succeeds (already covered by tls_keys)
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

static int sentinel = 0xc0ffee;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

int main(void)
{
	pthread_key_t key;
	void *got;

	h2_handle_errors(1);
	puts("Starting neg_tls_use_after_delete");

	if (pthread_key_create(&key, NULL) != 0) FAIL("key_create");
	if (pthread_setspecific(key, &sentinel) != 0) FAIL("setspecific pre-delete");
	got = pthread_getspecific(key);
	if (got != &sentinel) FAIL("getspecific pre-delete mismatch");

	if (pthread_key_delete(key) != 0) FAIL("key_delete");

	/* impl does not validate -- both calls succeed and value is observable */
	got = pthread_getspecific(key);
	if (got != &sentinel) FAIL("getspecific post-delete: value disappeared unexpectedly");

	if (pthread_setspecific(key, NULL) != 0) FAIL("setspecific post-delete");
	got = pthread_getspecific(key);
	if (got != NULL) FAIL("getspecific post-delete after NULL set");

	puts("TEST PASSED");
	return 0;
}

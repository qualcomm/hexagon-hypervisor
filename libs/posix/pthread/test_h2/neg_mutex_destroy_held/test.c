/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_mutex_destroy on a held mutex.
 *
 * The current impl of pthread_mutex_destroy is a no-op (returns 0
 * unconditionally). This test asserts:
 *   - destroy returns 0 even on a held mutex
 *   - the held state is preserved (we can still unlock it)
 *   - a subsequent lock/unlock cycle works on the same storage
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

int main(void)
{
	h2_handle_errors(1);
	puts("Starting neg_mutex_destroy_held");

	if (pthread_mutex_lock(&mtx) != 0) FAIL("base lock");

	if (pthread_mutex_destroy(&mtx) != 0) FAIL("destroy held returned non-zero");

	if (pthread_mutex_unlock(&mtx) != 0) FAIL("unlock after destroy");

	if (pthread_mutex_lock(&mtx) != 0) FAIL("relock");
	if (pthread_mutex_unlock(&mtx) != 0) FAIL("re-unlock");

	puts("TEST PASSED");
	return 0;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_join happy-path: create -> join -> verify retval round-trip for
 * NULL, a heap pointer, an integer-cast retval. Also a "join after the
 * worker has already finished" path (worker posts waiters then blocks in
 * `joined` -- main joins after a settle delay).
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

#define SETTLE_SPINS (1024*1024)

static int dummy_int = 0xdeadbeef;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *return_null(void *arg) { (void)arg; return NULL; }
static void *return_arg(void *arg) { return arg; }
static void *return_int_cast(void *arg) { (void)arg; return (void *)0xdeadbeef; }

int main(void)
{
	pthread_t t;
	void *retval;
	int i;

	h2_handle_errors(1);
	puts("Starting join_basic");

	if (pthread_create(&t, NULL, return_null, NULL) != 0) FAIL("create null");
	if (pthread_join(t, &retval) != 0) FAIL("join null");
	if (retval != NULL) FAIL("retval not NULL");

	if (pthread_create(&t, NULL, return_arg, &dummy_int) != 0) FAIL("create arg");
	if (pthread_join(t, &retval) != 0) FAIL("join arg");
	if (retval != &dummy_int) FAIL("retval mismatch (arg)");

	if (pthread_create(&t, NULL, return_int_cast, NULL) != 0) FAIL("create int");
	if (pthread_join(t, &retval) != 0) FAIL("join int");
	if (retval != (void *)0xdeadbeef) FAIL("retval mismatch (int cast)");

	/* join with NULL retval pointer */
	if (pthread_create(&t, NULL, return_null, NULL) != 0) FAIL("create nullret");
	if (pthread_join(t, NULL) != 0) FAIL("join nullret");

	/* join after worker has already entered pthread_exit */
	if (pthread_create(&t, NULL, return_int_cast, NULL) != 0) FAIL("create slow");
	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");
	if (pthread_join(t, &retval) != 0) FAIL("join slow");
	if (retval != (void *)0xdeadbeef) FAIL("retval mismatch (slow)");

	puts("TEST PASSED");
	return 0;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Same pthread_join happy-path as join_basic/test.c, but run with the guest
 * VM's physical base placed above 4GB (see this test's Makefile). Exercises
 * booter's aperture/bounce-buffer ELF load path and LINEAR guest translation
 * for a real multi-threaded guest at a high physical address, not just a
 * byte-correctness load check.
 *
 * NOTE: not yet wired into any scripts/testlist.v* -- see the Makefile
 * comment before enabling this in the regression.
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
	puts("Starting join_basic_4gb");

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

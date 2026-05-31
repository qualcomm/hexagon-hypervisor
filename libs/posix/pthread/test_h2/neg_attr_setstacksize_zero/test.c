/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_attr_setstacksize(0) negative test.
 *
 * Probe-only: this test verifies the *attribute* setter accepts 0 (the
 * current impl is permissive and stores it), but does NOT call
 * pthread_create with that attr because doing so would invoke the
 * trampoline on a 0-sized stack -> stack-overflow fault.
 *
 * If the impl is later updated to validate stacksize at attribute or
 * create time, this test should be expanded to verify the rejection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

int main(void)
{
	pthread_attr_t attr;
	size_t got;

	h2_handle_errors(1);
	puts("Starting neg_attr_setstacksize_zero (probe-only)");

	if (pthread_attr_init(&attr) != 0) FAIL("attr_init");

	/* current impl: setstacksize(0) succeeds (stores 0) */
	if (pthread_attr_setstacksize(&attr, 0) != 0)
		FAIL("setstacksize(0) returned non-zero");
	if (pthread_attr_getstacksize(&attr, &got) != 0) FAIL("getstacksize");
	if (got != 0) FAIL("stacksize was not 0 after setstacksize(0)");

	/* skip pthread_create -- would crash on a 0-sized stack */

	pthread_attr_destroy(&attr);

	puts("TEST PASSED");
	return 0;
}

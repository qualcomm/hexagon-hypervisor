/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_rwlock_unlock on an rwlock that was never locked. The current
 * rwlock impl is an ENOSYS stub for every entry point, so unlock always
 * returns ENOSYS and the test simply confirms that.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <errno.h>

static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

int main(void)
{
	int r;

	h2_handle_errors(1);
	puts("Starting neg_rwlock_unlock_unheld");

	r = pthread_rwlock_unlock(&rw);
	if (r != ENOSYS) FAIL("unlock-unheld did not return ENOSYS");

	puts("TEST PASSED");
	return 0;
}

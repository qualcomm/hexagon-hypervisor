/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Originally intended as a "writer stuck in pthread_rwlock_wrlock" reaper
 * test, but the current pthread_rwlock implementation
 * (libs/posix/pthread/rwlock/pthread_rwlock.ref.S) is a stub that
 * unconditionally returns ENOSYS for every entry point. See the sibling
 * stuck_in_rwlock_rd test for context.
 *
 * This test additionally verifies that the rdlock entry point is also a
 * stub -- a separate program from stuck_in_rwlock_rd in case future test
 * isolation matters.
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
	puts("Starting stuck_in_rwlock_wr (expected: rwlock impl is ENOSYS stub)");

	r = pthread_rwlock_rdlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_rdlock did not return ENOSYS");

	r = pthread_rwlock_wrlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_wrlock did not return ENOSYS");

	r = pthread_rwlock_unlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_unlock did not return ENOSYS");

	puts("TEST PASSED");
	return 0;
}

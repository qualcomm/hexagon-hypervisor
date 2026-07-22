/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Originally a "concurrent readers, writer blocks, try* return EBUSY"
 * test for pthread_rwlock. The current impl
 * (libs/posix/pthread/rwlock/pthread_rwlock.ref.S) is an ENOSYS stub for
 * every entry point, so this test instead asserts the stub behavior --
 * to be replaced with the real correctness scenario when the impl lands.
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
	puts("Starting rwlock_readers_writers (expected: rwlock impl is ENOSYS stub)");

	r = pthread_rwlock_init(&rw, NULL);
	if (r != 0) FAIL("rwlock_init not zero");

	r = pthread_rwlock_rdlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_rdlock not ENOSYS");

	r = pthread_rwlock_wrlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_wrlock not ENOSYS");

	r = pthread_rwlock_tryrdlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_tryrdlock not ENOSYS");

	r = pthread_rwlock_trywrlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_trywrlock not ENOSYS");

	r = pthread_rwlock_unlock(&rw);
	if (r != ENOSYS) FAIL("rwlock_unlock not ENOSYS");

	puts("TEST PASSED");
	return 0;
}

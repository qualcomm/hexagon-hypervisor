/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_attr_t getter/setter round-trips and unsupported-attribute
 * return codes (per libs/posix/pthread/thread/pthread_thread.ref.c):
 *   - schedpolicy / inheritsched return ENOTSUP
 *   - stacksize, stack, stackaddr, schedparam, detachstate, extra_np all
 *     round-trip via their {get,set} pair.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <errno.h>

static char dummy_stack[16384] __attribute__((aligned(8)));

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void noop_ctor(void *v) { (void)v; }
static void noop_dtor(void *v) { (void)v; }

int main(void)
{
	pthread_attr_t attr;
	size_t sz, got_sz;
	void *ptr, *got_ptr;
	struct sched_param sp, got_sp;
	int ds;
	int r;
	void *extra;
	void (*c)(void *), (*d)(void *);

	h2_handle_errors(1);
	puts("Starting attr_roundtrip");

	if (pthread_attr_init(&attr) != 0) FAIL("attr_init");

	/* unsupported -> ENOTSUP */
	r = pthread_attr_setschedpolicy(&attr, 0);
	if (r != ENOTSUP) FAIL("setschedpolicy not ENOTSUP");
	r = pthread_attr_getschedpolicy(&attr, &ds);
	if (r != ENOTSUP) FAIL("getschedpolicy not ENOTSUP");
	r = pthread_attr_setinheritsched(&attr, 0);
	if (r != ENOTSUP) FAIL("setinheritsched not ENOTSUP");
	r = pthread_attr_getinheritsched(&attr, &ds);
	if (r != ENOTSUP) FAIL("getinheritsched not ENOTSUP");

	/* stacksize round-trip */
	if (pthread_attr_setstacksize(&attr, 32768) != 0) FAIL("setstacksize");
	if (pthread_attr_getstacksize(&attr, &got_sz) != 0) FAIL("getstacksize");
	if (got_sz != 32768) FAIL("stacksize roundtrip mismatch");

	/* stackaddr round-trip */
	if (pthread_attr_setstackaddr(&attr, dummy_stack) != 0) FAIL("setstackaddr");
	if (pthread_attr_getstackaddr(&attr, &got_ptr) != 0) FAIL("getstackaddr");
	if (got_ptr != dummy_stack) FAIL("stackaddr roundtrip mismatch");

	/* stack pair: setstack/getstack */
	sz = sizeof(dummy_stack);
	ptr = dummy_stack;
	if (pthread_attr_setstack(&attr, ptr, sz) != 0) FAIL("setstack");
	if (pthread_attr_getstack(&attr, &got_ptr, &got_sz) != 0) FAIL("getstack");
	if (got_ptr != ptr) FAIL("setstack/getstack ptr mismatch");
	if (got_sz != sz) FAIL("setstack/getstack size mismatch");

	/* schedparam round-trip */
	sp.sched_priority = 123;
	if (pthread_attr_setschedparam(&attr, &sp) != 0) FAIL("setschedparam");
	if (pthread_attr_getschedparam(&attr, &got_sp) != 0) FAIL("getschedparam");
	if (got_sp.sched_priority != 123) FAIL("schedparam roundtrip");

	/* detachstate round-trip */
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) FAIL("setdetachstate");
	if (pthread_attr_getdetachstate(&attr, &ds) != 0) FAIL("getdetachstate");
	if (ds != PTHREAD_CREATE_DETACHED) FAIL("detachstate roundtrip");

	/* extra_np round-trip */
	if (pthread_attr_setextra_np(&attr, dummy_stack, noop_ctor, noop_dtor) != 0)
		FAIL("setextra_np");
	if (pthread_attr_getextra_np(&attr, &extra, &c, &d) != 0)
		FAIL("getextra_np");
	if (extra != dummy_stack || c != noop_ctor || d != noop_dtor)
		FAIL("extra_np roundtrip");

	if (pthread_attr_destroy(&attr) != 0) FAIL("attr_destroy");

	puts("TEST PASSED");
	return 0;
}

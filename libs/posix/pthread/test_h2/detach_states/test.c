/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_detach state matrix:
 *   - detach a live thread before exit (success)
 *   - detach an unknown TID -> ESRCH
 *   - detach an already-detached thread -> EINVAL
 *
 * The "detach a thread that already posted waiters" case is exercised
 * implicitly by the live-detach path; we keep coverage of it by adding a
 * settle delay before issuing pthread_detach.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <errno.h>

#define SETTLE_SPINS (1024*1024)

static h2_sem_t go;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *blocked_until_go(void *arg)
{
	(void)arg;
	h2_sem_down(&go);
	return NULL;
}

static void *quick(void *arg) { (void)arg; return NULL; }

int main(void)
{
	pthread_t t;
	int r;
	int i;

	h2_sem_init_val(&go, 0);
	h2_handle_errors(1);
	puts("Starting detach_states");

	/* detach a live thread, then unblock it */
	if (pthread_create(&t, NULL, blocked_until_go, NULL) != 0) FAIL("create live");
	if (pthread_detach(t) != 0) FAIL("detach live");
	h2_sem_up(&go);

	/* unknown TID */
	r = pthread_detach((pthread_t)0xdeadbeef);
	if (r != ESRCH) FAIL("unknown TID did not return ESRCH");

	/* double-detach: detach a thread, then detach again */
	if (pthread_create(&t, NULL, blocked_until_go, NULL) != 0) FAIL("create dbl");
	if (pthread_detach(t) != 0) FAIL("first detach");
	r = pthread_detach(t);
	if (r != EINVAL) FAIL("double-detach did not return EINVAL");
	h2_sem_up(&go);

	/* detach a thread that has likely already posted waiters */
	if (pthread_create(&t, NULL, quick, NULL) != 0) FAIL("create slow-detach");
	for (i = 0; i < SETTLE_SPINS; i++) asm volatile ("nop");
	if (pthread_detach(t) != 0) FAIL("detach after waiters posted");

	puts("TEST PASSED");
	return 0;
}

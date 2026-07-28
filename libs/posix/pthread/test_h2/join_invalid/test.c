/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_join error paths:
 *   - join on an unknown TID -> ESRCH
 *   - join on a detached thread -> EINVAL
 *   - join on a thread that has already been joined (TCB removed) -> ESRCH
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <errno.h>

static h2_sem_t go;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *quick_return(void *arg) { (void)arg; return NULL; }

static void *blocked_until_go(void *arg)
{
	(void)arg;
	h2_sem_down(&go);
	return NULL;
}

int main(void)
{
	pthread_t t;
	int r;

	h2_sem_init_val(&go, 0);
	h2_handle_errors(1);
	puts("Starting join_invalid");

	/* unknown TID */
	r = pthread_join((pthread_t)0xdeadbeef, NULL);
	if (r != ESRCH) FAIL("unknown TID did not return ESRCH");

	/* detached thread */
	if (pthread_create(&t, NULL, blocked_until_go, NULL) != 0) FAIL("create detach");
	if (pthread_detach(t) != 0) FAIL("detach");
	r = pthread_join(t, NULL);
	if (r != EINVAL) FAIL("detached did not return EINVAL");
	h2_sem_up(&go);	/* let detached thread finish */

	/* already-joined thread (TCB has been removed) */
	if (pthread_create(&t, NULL, quick_return, NULL) != 0) FAIL("create dbljoin");
	if (pthread_join(t, NULL) != 0) FAIL("first join");
	r = pthread_join(t, NULL);
	if (r != ESRCH) FAIL("second join did not return ESRCH");

	puts("TEST PASSED");
	return 0;
}

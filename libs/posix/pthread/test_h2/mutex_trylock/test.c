/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_mutex_trylock corner cases:
 *   - uncontended trylock returns 0 and acquires
 *   - trylock when held by another thread returns EBUSY
 *   - trylock on a recursive mutex held by self returns 0 and bumps depth
 *   - trylock on a NORMAL mutex held by self returns EBUSY (per impl)
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>
#include <errno.h>

static pthread_mutex_t mtx_n = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_r = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static h2_sem_t holder_in;
static h2_sem_t go;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

static void *holder(void *arg)
{
	pthread_mutex_t *m = arg;
	pthread_mutex_lock(m);
	h2_sem_up(&holder_in);
	h2_sem_down(&go);
	pthread_mutex_unlock(m);
	return NULL;
}

int main(void)
{
	pthread_t t;
	int r;

	h2_sem_init_val(&holder_in, 0);
	h2_sem_init_val(&go, 0);
	h2_handle_errors(1);
	puts("Starting mutex_trylock");

	/* uncontended */
	if (pthread_mutex_trylock(&mtx_n) != 0) FAIL("uncontended trylock");
	if (pthread_mutex_unlock(&mtx_n) != 0) FAIL("unlock after uncontended");

	/* contended -> EBUSY */
	if (pthread_create(&t, NULL, holder, &mtx_n) != 0) FAIL("create holder");
	h2_sem_down(&holder_in);
	r = pthread_mutex_trylock(&mtx_n);
	if (r != EBUSY) FAIL("contended did not return EBUSY");
	h2_sem_up(&go);
	if (pthread_join(t, NULL) != 0) FAIL("join holder");

	/* recursive: trylock-self succeeds and bumps depth */
	if (pthread_mutex_lock(&mtx_r) != 0) FAIL("recursive base lock");
	if (pthread_mutex_trylock(&mtx_r) != 0) FAIL("recursive trylock-self");
	if (pthread_mutex_unlock(&mtx_r) != 0) FAIL("recursive unlock 1");
	if (pthread_mutex_unlock(&mtx_r) != 0) FAIL("recursive unlock 2");

	/* normal: trylock-self returns EBUSY */
	if (pthread_mutex_lock(&mtx_n) != 0) FAIL("normal base lock");
	r = pthread_mutex_trylock(&mtx_n);
	if (r != EBUSY) FAIL("normal trylock-self did not return EBUSY");
	if (pthread_mutex_unlock(&mtx_n) != 0) FAIL("normal unlock");

	puts("TEST PASSED");
	return 0;
}

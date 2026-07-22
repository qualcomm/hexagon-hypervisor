/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_barrier_init with count=0.
 *
 * Probe-only: count=0 is undefined per POSIX. The current impl stores
 * threads_left = 0 and threads_total = 0. Behavior of barrier_wait on
 * such a barrier is unclear (an arriving thread sees threads_left = 0,
 * which is the "all arrived" condition, so it might immediately release
 * with SERIAL_THREAD).
 *
 * To avoid a hang on a misbehaving impl, this test only exercises init
 * and destroy on a count=0 barrier, not wait.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

int main(void)
{
	pthread_barrier_t b;
	int r;

	h2_handle_errors(1);
	puts("Starting neg_barrier_init_zero (probe-only)");

	r = pthread_barrier_init(&b, NULL, 0);
	(void)r;	/* impl-defined; just verify it does not crash */

	r = pthread_barrier_destroy(&b);
	(void)r;	/* impl-defined */

	puts("TEST PASSED");
	return 0;
}

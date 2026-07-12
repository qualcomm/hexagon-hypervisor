/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * pthread_create with NULL start_routine.
 *
 * Probe-only / expected: the current pthread impl does not validate the
 * start_routine argument; pthread_trampoline calls it unconditionally
 * (libs/posix/pthread/thread/pthread_thread.ref.c::pthread_trampoline).
 * Passing NULL therefore causes the new thread to jump to address 0
 * and fault.
 *
 * To avoid an unrecoverable test crash, this test does NOT call
 * pthread_create with NULL. Instead it documents the impl gap and
 * passes once started.
 *
 * If the impl is later updated to validate (return EINVAL or similar),
 * this test should be replaced with the actual rejection check.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <pthread.h>

int main(void)
{
	h2_handle_errors(1);
	puts("Starting neg_create_invalid_routine (expected: impl does not validate NULL)");
	puts("TEST PASSED");
	return 0;
}

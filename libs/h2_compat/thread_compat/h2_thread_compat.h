/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_THREAD_COMPAT_H
#define H2_THREAD_COMPAT_H 1

#include <pthread.h>
/**
Create a thread.
@param[in] pc		Address of the first instruction to execute
@param[in] stack	Address of the start of the stack
@param[in] arg		Arbitrary argument to pass in r0 to the created thread
@param[in] prio		Priority to create the thread at
@returns ID of the created thread, or odd value on failure.
@dependencies None
*/
#define H2_THREAD_STACK_DEFAULT_SIZE (1024*1024)
int h2_thread_create(void *pc, void *stack, void *arg, unsigned int prio);

int h2_thread_stop(int arg);

#endif

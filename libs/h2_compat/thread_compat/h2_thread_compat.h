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
static inline int h2_thread_create(void *pc, void *stack, void *arg, unsigned int prio)
{
	pthread_attr_t attr;
	pthread_t tid;
	int ret;
	struct sched_param sched;
	unsigned char *stackstart;
	stackstart = stack;
	stackstart -= H2_THREAD_STACK_DEFAULT_SIZE;
	sched.sched_priority = prio;
	pthread_attr_init(&attr);
	pthread_attr_setschedparam(&attr,&sched);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstack(&attr,stackstart,H2_THREAD_STACK_DEFAULT_SIZE);
	if ((ret = pthread_create(&tid,&attr,pc,arg)) != 0) {
		return -1;
	} else {
		return (int)tid;
	}
}

static inline int h2_thread_stop(int arg)
{
	pthread_exit((void *)arg);
}

#endif

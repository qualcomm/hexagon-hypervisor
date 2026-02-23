/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_thread_compat.ref.c
 * @brief Thread compatibility functions - Implementation
 */

#include "h2_thread_compat.h"

int h2_thread_create(void *pc, void *stack, void *arg, unsigned int prio)
{
	pthread_attr_t attr;
	pthread_t tid;
	int ret;
	struct sched_param sched;
	unsigned char *stackstart;
	stackstart = (unsigned char *)stack;
	stackstart -= H2_THREAD_STACK_DEFAULT_SIZE;
	sched.sched_priority = prio;
	pthread_attr_init(&attr);
	pthread_attr_setschedparam(&attr,&sched);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstack(&attr,stackstart,H2_THREAD_STACK_DEFAULT_SIZE);
	if ((ret = pthread_create(&tid,&attr,(void *(*)(void *))pc,arg)) != 0) {
		return -1;
	} else {
		return (int)tid;
	}
}

int h2_thread_stop(int arg)
{
	pthread_exit((void *)arg);
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <qurt.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	/*
	 * This should just call qurt create, qurt and posix should be compatible.
	 * This means that qurt thread create should be copying the TLS areas.
	 */
	/* EJP: we should try and make qurt thread creation simpler for common cases */
	static const pthread_attr_t pthread_attr_default = { PTHREAD_DEFAULT_STACKSIZE, NULL, { 100 }};
	qurt_thread_attr_t qurt_attr;
	qurt_thread_t tid;
	if (attr == NULL) attr = &pthread_attr_default;
	qurt_thread_attr_init(&qurt_attr);
	qurt_thread_attr_set_name(&qurt_attr,"pthread");
	qurt_thread_attr_set_stack_size(&qurt_attr, attr->stacksize);
	qurt_thread_attr_set_stack_addr(&qurt_attr, attr->stackaddr);
	qurt_thread_attr_set_priority(&qurt_attr, attr->sched.sched_priority);
	qurt_thread_attr_set_timetest_id(&qurt_attr, attr->sched.sched_priority);
	if ((tid = qurt_thread_create(&tid,&qurt_attr,start_routine,arg)) == QURT_EFATAL) {
		return EAGAIN;
	};
	return 0;
}


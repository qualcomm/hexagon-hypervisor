/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>

int pthread_join(pthread_t thread, void **retval)
{
	int qurt_status;
	int qurt_err;
	if ((qurt_err = qurt_thread_join(thread, &qurt_status)) != 0) {
		return qurt_err;
	}
	if (retval) *retval = (void *)((long)(qurt_status));
	return 0;
} 


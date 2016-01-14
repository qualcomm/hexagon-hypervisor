/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>

int pthread_detach(pthread_t thread)
{
	/*
 	 * pthread_detach makes the target thread non-joinable
	 */
	return 0;
}


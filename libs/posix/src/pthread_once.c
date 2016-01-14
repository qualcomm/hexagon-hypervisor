/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <qurt.h>
#include <qurt_atomic_ops.h>

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
	if (qurt_atomic_compare_and_set(once_control,0,1) == 1) {
		init_routine();
	}
	return 0;
}


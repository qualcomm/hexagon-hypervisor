/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <h2if.h>

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
	if (h2_atomic_compare_swap32(once_control,0,1) == 0) {
		init_routine();
	}
	return 0;
}


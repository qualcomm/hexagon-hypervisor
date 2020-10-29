/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <pthread.h>

void __h2_default_thread_stop_hook__(int status)
{
	pthread_exit((void *)status);
}

void __h2_thread_stop_hook__(int status) __attribute__ ((weak,alias("__h2_default_thread_stop_hook__")));

void sys_exit(okay_t status)
{

	if (0 == status && (void (*)(int))0xfffffff0 != __h2_thread_stop_hook__) {
		__h2_thread_stop_hook__(status);
	}

	ANGEL(SYS_EXIT,status,status);
}

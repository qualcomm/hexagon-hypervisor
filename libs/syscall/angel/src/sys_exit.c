/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <h2.h>

void __attribute__ ((weak)) __h2_thread_stop_hook__(int status)
{
	h2_thread_stop(status);
}

void sys_exit(okay_t status)
{

	if (0 == status && NULL != __h2_thread_stop_hook__) {
		__h2_thread_stop_hook__(status);
	}

	clean(&status, 1);
	ANGEL(SYS_EXIT,&status,status);
}

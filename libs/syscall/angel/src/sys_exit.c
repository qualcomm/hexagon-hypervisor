/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <pthread.h>
#include <h2_vm.h>
#include <h2_common_vm.h>

void __h2_default_thread_stop_hook__(int status)
{
	pthread_exit((void *)status);
}

void __h2_thread_stop_hook__(int status) __attribute__ ((weak,alias("__h2_default_thread_stop_hook__")));

void sys_exit(okay_t status)
{
	/* Sentinel hook (0xfffffff0) means we're in a standalone build (no h2
	 * hypervisor underneath, e.g. kernel-level tests).  TRAP1-based h2
	 * calls would go nowhere, so skip the vm reap and the pthread_exit
	 * hook. */
	if ((void (*)(int))0xfffffff0 != __h2_thread_stop_hook__) {
		h2_vmkill(VMOP_KILL_VM_SELF, (int)status);

		if (0 == status) {
			__h2_thread_stop_hook__(status);
		}
	}

	ANGEL(SYS_EXIT,status,status);
}

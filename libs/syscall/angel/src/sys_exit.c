/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <pthread.h>

/* Forward declaration: h2_vm_stop_trap lives in libh2 (libs/h2/thread/h2_thread.h)
 * which is not on this include path. */
void h2_vm_stop_trap(int status) __attribute__((noreturn));

void __h2_default_thread_stop_hook__(int status)
{
	/* POSIX exit(): tear down the entire VM via the kernel.  Works on real
	 * silicon (no ANGEL required).  Tests that want the simulator
	 * to terminate the whole process can still link with
	 *   LDFLAGS += -Wl,--defsym=__h2_thread_stop_hook__=0xfffffff0
	 * which causes sys_exit to skip the hook and fall through to ANGEL. */
	h2_vm_stop_trap(status);
}

void __h2_thread_stop_hook__(int status) __attribute__ ((weak,alias("__h2_default_thread_stop_hook__")));

void sys_exit(okay_t status)
{

	if ( (void (*)(int))0xfffffff0 != __h2_thread_stop_hook__) {
		__h2_thread_stop_hook__(status);
	}

	ANGEL(SYS_EXIT,status,status);
}

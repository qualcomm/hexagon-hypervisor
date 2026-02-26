/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <h2_error.h>
#include <syscall_defs.h>

void __attribute__ ((weak)) __h2_handle_errors_hook__(void) {
	h2_handle_errors(0);
}

char __boot_cmdline__[SIZE__boot_cmdline__] __attribute__((section(".data"))) = { 0 };

static errno_t do_sys_get_cmdline(char *buffer, count_t count) {
	errno_t ret;
	struct { char *buf; count_t count; } x;

	if ((void (*)(void))0xfffffff0 != __h2_handle_errors_hook__) {
		__h2_handle_errors_hook__();
	}

	if (__boot_cmdline__[0] != 0) {
		strncpy(buffer,__boot_cmdline__,count);
		buffer[count-1] = 0;
		return 0;
	}
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.count = count;
	clean(buffer,count/4+3); clean(&x,2);
	ret = ANGEL(SYS_GET_CMDLINE,&x,0);
	clean(buffer,count/4+3);
	return ret;
}

/* Trampoline to align sp down to next cache line. This prevents stack
	 allocations further down the call tree from falling in the same
	 cache line as the buffer that needs to be invalidated prior to
	 angel SYS_READ, which can foil the invalidation. */
errno_t sys_get_cmdline(char *buffer, count_t count) {
	unsigned long sp_save;
	/*FIXME: Maybe we should get the actual cache line size from cfg_table and save it in a global */
	asm volatile ( " %0 = r29  // save sp \n"
								 " r29 = and(r29, #-256)  // align sp down \n"
								 : "=r"(sp_save));
	return do_sys_get_cmdline(buffer, count);
}

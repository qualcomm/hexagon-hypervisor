/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <h2_error.h>

void __attribute__ ((weak)) __h2_handle_errors_hook__(void) {
	h2_handle_errors(0);
}

char __boot_cmdline__[256] __attribute__((section(".data"))) = { 0 };

errno_t sys_get_cmdline(char *buffer, count_t count)
{
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


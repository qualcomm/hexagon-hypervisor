/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_isatty_internal(int fd) {
	sys_call_ret_t ret;
	clean(&fd, 1);
	ret = angel_with_err(SYS_ISTTY,&fd, 0);
	DEBUG_PRINTF("sys_istty: fd=%d ret=%d\n", fd,r et.ret_value);
	return ret;
}

int sys_isatty() { return 0; }


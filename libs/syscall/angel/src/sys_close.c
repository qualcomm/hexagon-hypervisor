/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_close_internal(fd_t fd) {
	clean(&fd,1);
	return angel_with_err(SYS_CLOSE, &fd, 0);
}

errno_t sys_close(fd_t fd) {
	return sys_close_internal(fd).ret_value;
}

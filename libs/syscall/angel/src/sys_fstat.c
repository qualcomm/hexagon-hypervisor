/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_fstat_internal(fd_t fd, void *buffer)
{
	struct {
		fd_t fd;
		void *buf;
	} x;
	x.fd = fd;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	clean(buffer, sizeof(struct __sys_stat) / 4);
	clean(&x, 2);
	return angel_with_err(SYS_FSTAT, &x, 0);
}

errno_t sys_fstat(fd_t fd, void *buffer) {
	return sys_fstat_internal(fd, buffer).ret_value;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_seek_internal(fd_t fd, count_t offset) {
	sys_call_ret_t  ret;
	struct { fd_t fd; count_t c; } x; x.fd = fd; x.c = offset;
	clean(&x,2);
	ret = angel_with_err(SYS_SEEK,&x,0);
	DEBUG_PRINTF("sys_seek: fd: %d off: %d ret: %d\n",fd,offset,ret.ret_value);
	return ret;
}

errno_t sys_seek(fd_t fd, count_t offset) {
	return sys_seek_internal(fd, offset).ret_value
			;
}

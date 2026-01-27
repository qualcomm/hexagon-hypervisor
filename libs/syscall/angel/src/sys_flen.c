/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_flen_internal(fd_t fd) {
	sys_call_ret_t ret;
	clean(&fd,1);
	ret = angel_with_err(SYS_FLEN,&fd,0);
	DEBUG_PRINTF("sys_flen: fd=%d ret=%d\n",fd,ret.ret_value);
	return ret;
}

errno_t sys_flen(fd_t fd) {
	return sys_flen_internal(fd).ret_value;
}

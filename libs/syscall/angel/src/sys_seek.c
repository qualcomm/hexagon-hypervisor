/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

errno_t sys_seek(fd_t fd, count_t offset)
{
	errno_t ret;
	struct { fd_t fd; count_t c; } x; x.fd = fd; x.c = offset;
	clean(&x,2);
	ret = ANGEL(SYS_SEEK,&x,0);
	DEBUG_PRINTF("sys_seek: fd: %d off: %d ret: %d\n",fd,offset,ret);
	return ret;
}


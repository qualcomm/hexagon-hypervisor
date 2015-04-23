/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

count_t sys_read(fd_t fd, char *buffer, count_t count)
{
	count_t ret;
	struct { fd_t fd; char *buf; count_t count; } x;
	unsigned long start,end;
	start = ALIGN32_DOWN((unsigned long)buffer);
	end = ALIGN32_UP((unsigned long)buffer + count);
	invalidate((void *)start, end-start);
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.fd = fd;
	x.count = count;
	clean(&x,3);
	ret = ANGEL(SYS_READ,&x,0);
	return ret;
}


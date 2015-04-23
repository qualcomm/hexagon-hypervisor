/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

count_t sys_write(fd_t fd, const char *buffer, count_t count)
{
	struct { fd_t fd; const char *buf; count_t c; } x;
	x.fd = fd;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.c = count;
	clean(buffer,count/4+3); clean(&x,3); return ANGEL(SYS_WRITE,&x,0);
}


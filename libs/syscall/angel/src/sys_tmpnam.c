/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

errno_t sys_tmpnam(char *buffer, unsigned char target, count_t count)
{
	errno_t ret;
	struct { char *buf; unsigned char target; count_t c; } x;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.target = target; x.c = count;
	clean(buffer,count/4+3); clean(&x,3); ret = ANGEL(SYS_TMPNAM,&x,0);
	clean(buffer,count/4+3);
	return ret;
}

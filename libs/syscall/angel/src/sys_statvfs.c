/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

int sys_statvfs(char *rootdir, void *buffer)  /* Clean buffer? */
{
	struct {
		char *rd;
		void *buf;
	} x;
	x.rd = ANGEL_OFFSET_PTR(rootdir);
	x.buf = ANGEL_OFFSET_PTR(buffer);
	clean(buffer, sizeof(struct __sys_stat) / 4);
	clean_str(rootdir);
	clean(&x, 2);
	return ANGEL(SYS_FSTATVFS,&x,0);
}


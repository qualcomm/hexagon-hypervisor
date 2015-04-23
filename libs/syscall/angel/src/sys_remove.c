/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

okay_t sys_remove(const char *name)
{
	struct { const char *name; int len; } x;
	x.name = ANGEL_OFFSET_PTR(name); x.len = strlen(name);
	clean_str(name); clean(&x,2);
	return ANGEL(SYS_REMOVE,&x,0);
}


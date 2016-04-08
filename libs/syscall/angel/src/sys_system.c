/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

okay_t sys_system(const char *command)
{
	struct {
		const char *cmd;
		int cl;
	} x;
	x.cmd = ANGEL_OFFSET_PTR(command);
	x.cl = strlen(x.cmd);
	clean_str(command);
	clean(&x,2);
	return ANGEL(SYS_SYSTEM, &x, 0);
}


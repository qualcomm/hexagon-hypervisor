/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

struct dirent *sys_readdir(int dir, struct dirent *dptr) {
	clean(dptr, sizeof(struct dirent) / 4);
	return VANGEL(SYS_READDIR,dir,ANGEL_OFFSET_PTR(dptr));
}


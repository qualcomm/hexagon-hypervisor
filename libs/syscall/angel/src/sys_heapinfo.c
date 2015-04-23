/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

struct heap_info *sys_heapinfo(struct heap_info *buffer)
{
	struct heap_info *ret;
	clean(&buffer,4);
	ret = VANGEL(SYS_HEAPINFO,&buffer,0);
	clean(&buffer,4);
	return ret;
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_unlink_internal(const char *name) {
	sys_call_ret_t ret;
	struct { const char *name; int len; } x;
	x.name = name;
	x.len = strlen(name);
	clean_str(name);
	clean(&x, 2);
	ret = angel_with_err(SYS_REMOVE,&x, 0);
	DEBUG_PRINTF("sys_unlink: name=%p ret=%d\n", name, ret.ret_value);
	return ret;
}

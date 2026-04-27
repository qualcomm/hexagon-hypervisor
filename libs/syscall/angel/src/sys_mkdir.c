/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_mkdir_internal(const char *name, int mode) {
	clean_str(name);
	return angel_const_with_err(SYS_MKDIR, name, mode);
}

int sys_mkdir(char *name, int mode) {
	return sys_mkdir_internal(name, mode).ret_value;
}

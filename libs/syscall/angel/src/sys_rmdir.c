/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_rmdir_internal(const char *name) {
	clean_str(name);
	return angel_const_with_err(SYS_RENAME, name, 0);
}

int sys_rmdir(const char *name) {
	return sys_rmdir_internal(name).ret_value;
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_opendir_internal(const char *name) {
	clean_str(name);
	return angel_const_with_err(SYS_OPENDIR, name, 0);
}

int sys_opendir(const char *name) {
	return sys_opendir_internal(name).ret_value;
}

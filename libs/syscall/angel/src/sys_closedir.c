/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_closedir_internal(int dir) {
	return angel_with_err(SYS_CLOSEDIR, (void *)(uintptr_t)dir, 0);
}

errno_t sys_closedir(int dir) {
	return sys_closedir_internal(dir).ret_value;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_readdir_internal(int dir, dirent_internal *dptr) {
	clean(dptr, sizeof(dirent_internal) / 4);
	return angel_with_err(SYS_READDIR, (void *)(uintptr_t)dir, (unsigned int)ANGEL_OFFSET_PTR(dptr));
}

dirent_internal *sys_readdir(int dir, dirent_internal *dptr) {
	return (dirent_internal *)(uintptr_t)sys_readdir_internal(dir, dptr).ret_value;
}

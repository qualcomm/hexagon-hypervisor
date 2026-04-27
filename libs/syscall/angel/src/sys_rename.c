/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_rename_internal(const char *oldname, const char *newname)
{
	struct { const char *on; int ol; const char *nn; int nl; } x;
 	x.on = ANGEL_OFFSET_PTR(oldname); x.ol = strlen(oldname);
	x.nn = ANGEL_OFFSET_PTR(newname); x.nl = strlen(newname);
	clean_str(oldname); clean_str(newname); clean(&x,4);
	return angel_with_err(SYS_RENAME,&x,0);
}

okay_t sys_rename(const char *oldname, const char *newname) {
	return sys_rename_internal(oldname, newname).ret_value;
}

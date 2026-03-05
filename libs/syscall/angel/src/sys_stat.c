/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

sys_call_ret_t sys_stat_internal(const char *name, void *buffer) {
	struct {
		char *n;
		void *buf;
	} x;
	x.n = ANGEL_OFFSET_PTR(name);
	x.buf = buffer;
	clean_str(name);
	clean(buffer, sizeof(struct __sys_stat) / 4);
	clean(&x, 2);
	return angel_with_err(SYS_STAT, &x, 0);
}

errno_t sys_stat(const char *name, void *buffer) {
	return sys_stat_internal(name, buffer).ret_value;
}

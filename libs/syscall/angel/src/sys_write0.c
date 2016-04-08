/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

void sys_write0(const char *str) {
	clean_str(str);
	clean(&str, 1);
	ANGEL(SYS_WRITE0, &str, 0);
}


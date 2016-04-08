/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

void sys_writec(unsigned char ch) {
	clean(&ch, 1);
	ANGEL(SYS_WRITEC, &ch, 0);
}


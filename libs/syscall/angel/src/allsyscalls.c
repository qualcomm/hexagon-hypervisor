/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <angel.h>
#include <string.h>
#include <stdlib.h>
#include <syscall_defs.h>
#include <h2_error.h>
#include <h2_thread.h>

#define ALIGN32_DOWN(X) ((X) & -32)
#define ALIGN32_UP(X) (((X) + 31) & -32)

long __boot_net_phys_offset__;

unsigned int angel(unsigned int r0, void *r1, unsigned int r2) {
	return __angel(r0, ANGEL_OFFSET_PTR(r1), r2);
}


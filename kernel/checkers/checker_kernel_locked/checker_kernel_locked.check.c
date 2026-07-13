/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <checker_help.h>
#include <checker_kernel_locked.h>

s32_t checker_kernel_locked()
{
	if (!IS_BKL_LOCKED()) {
		FAIL("checker_kernel_locked: Kernel not locked.");
	}
	return 1;
}

s32_t checker_kernel_unlocked()
{
	if (IS_BKL_LOCKED()) {
		FAIL("checker_kernel_unlocked: Kernel is Locked.");
	}
	return 1;
}


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <checker_help.h>
#include <checker_kernel_locked.h>

void checker_kernel_locked()
{
#if __QDSP6_ARCH__ >= 3
	u32_t syscfg = H2K_get_syscfg();
	if ((syscfg & (1<<12)) == 0) {
		FAIL("Kernel not locked");
	}
#else
#warning fixme: look at bkl
#endif
}


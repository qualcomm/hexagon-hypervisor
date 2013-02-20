/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <checker_help.h>
#include <checker_tlb_locked.h>

s32_t checker_tlb_locked()
{
#if ARCHV >= 3
	u32_t syscfg = H2K_get_syscfg();
	if ((syscfg & (1<<11)) == 0) {
		FAIL("checker_kernel_locked: TLB not locked.");
	}
#else
#error support less than v3
#endif
	return 1;
}

s32_t checker_tlb_unlocked()
{
#if ARCHV >= 3
	u32_t syscfg = H2K_get_syscfg();
	if ((syscfg & (1<<11)) != 0) {
		FAIL("checker_kernel_unlocked: TLB is Locked.");
	}
#else
#error support less than v3
#endif
	return 1;
}


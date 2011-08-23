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
#if __QDSP6_ARCH__ >= 3
	u32_t syscfg = H2K_get_syscfg();
	if ((syscfg & (1<<11)) == 0) {
		FAIL("checker_kernel_locked: TLB not locked.");
	}
#else
#warning fixme: look at bkl
#endif
	return 1;
}

s32_t checker_tlb_unlocked()
{
#if __QDSP6_ARCH__ >= 3
	u32_t syscfg = H2K_get_syscfg();
	if ((syscfg & (1<<11)) != 0) {
		FAIL("checker_kernel_unlocked: TLB is Locked.");
	}
#else
#warning fixme: look at bkl
#endif
	return 1;
}


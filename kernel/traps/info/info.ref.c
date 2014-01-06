/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <globals.h>
#include <h2_common_info.h>

u32_t H2K_trap_info(u32_t op, H2K_thread_context *me) {

	u32_t val;

	switch(op) {

	case INFO_BUILD_ID:
		return H2K_gp->build_id;

	case INFO_BOOT_FLAGS:
		return H2K_gp->info_boot_flags.raw;

	case INFO_STLB:
		return H2K_gp->info_stlb.raw;

	case INFO_SYSCFG:
		asm volatile ( "%0 = syscfg\n" : "=r" (val));
		return val;

	case INFO_REV:
		asm volatile ( "%0 = rev\n" : "=r" (val));
		return val;

	default:
		return -1;
	}
}

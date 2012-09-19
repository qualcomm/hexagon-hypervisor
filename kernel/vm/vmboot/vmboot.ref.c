/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <create.h>
#include <globals.h>
#include <idtype.h>

s32_t H2K_vmboot(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me)
{
	return H2K_thread_create_no_squash(pc, sp, arg1, prio, H2K_gp->vmblocks[vm], me);
}

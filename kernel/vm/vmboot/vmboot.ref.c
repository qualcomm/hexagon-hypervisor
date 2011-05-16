/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <create.h>

s32_t H2K_vmboot(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	
	return H2K_thread_create(pc, sp, arg1, prio, vmblock, me);
}

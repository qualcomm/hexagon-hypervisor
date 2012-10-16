/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMOP_H
#define H2K_VMOP_H 1

#include <vm.h>
#include <context.h>

typedef enum {
	VMOP_BOOT,
	VMOP_STATUS,
	VMOP_FREE,
	VMOP_MAX
} vmop_t;

typedef enum {
	VMOP_STATUS_STATUS,
	VMOP_STATUS_CPUS,
	VMOP_STATUS_MAX
} vmop_status_t;

s32_t H2K_vmop_boot(vmop_t unused0, u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot");

s32_t H2K_vmop_status(vmop_t unused0, u32_t, u32_t vm, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot");

s32_t H2K_vmop_free(vmop_t unused0, u32_t vm, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot");

#endif

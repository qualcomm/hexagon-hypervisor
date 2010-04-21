/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_HWCONFIG_H
#define H2K_HWCONFIG_H 1

#include <c_std.h>
#include <context.h>

u32_t H2K_trap_hwconfig(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me);
u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me);
u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me);
u32_t H2K_trap_hwconfig_prefetch(u32_t unused, void *handler, u32_t unused2, u32_t unused3, H2K_thread_context *me);

enum {
	HWCONFIG_PREFETCH_HF_I,
	HWCONFIG_PREFETCH_HF_D,
	HWCONFIG_PREFETCH_SF_D,
	HWCONFIG_PREFETCH_HF_I_L2,
	HWCONFIG_PREFETCH_SF_D_L2
};

enum {
	HWCONFIG_PARTITION_D,
	HWCONFIG_PARTITION_I,
	HWCONFIG_PARTITION_L2
};

#endif


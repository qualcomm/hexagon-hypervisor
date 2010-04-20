/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <config.h>
#include <asm_offsets.h>
#include <thread.h>
#include <fatal.h>
#include <globals.h>
#include <stmode.h>

typedef u32_t (*configptr_t)(u32_t, void *, u32_t, u32_t, H2K_thread_context *);
#include <hw.h>

#define MAX_CONFIGS 3

static const configptr_t H2K_hwconfigtab[MAX_CONFIGS] = {
	H2K_trap_hwconfig_l2cache,
	H2K_trap_hwconfig_partitions,
	H2K_trap_hwconfig_prefetch,
};

u32_t H2K_trap_hwconfig(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)
{
	if (configtype >= MAX_CONFIGS) return 0;
	return H2K_hwconfigtab[configtype](0,ptr,val2,val3,me);
}

u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me)
{
	if (H2K_stmode_begin() != 0) return -1;
	/* Change size?  Change modality? Etc */
	H2K_stmode_end();
	return 0;
}

u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)
{
	return 0;
}

u32_t H2K_trap_hwconfig_prefetch(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)
{
	return 0;
}


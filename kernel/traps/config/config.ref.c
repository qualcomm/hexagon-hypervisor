/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <config.h>

typedef void (*configptr_t)(u32_t, void *, u32_t, u32_t, BLASTK_thread_context *);

#define MAX_CONFIGS 2

static configptr_t BLASTK_configtab[MAX_CONFIGS] = {
	BLASTK_trap_config_addthreads,
	BLASTK_trap_config_schedint,
};

void BLASTK_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, BLASTK_thread_context *me)
{
	if (configtype >= MAX_CONFIGS) return;
	BLASTK_configtab[configtype](0,ptr,val2,val3,me);
}

void BLASTK_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, BLASTK_thread_context *me)
{
}

void BLASTK_trap_config_schedint(u32_t unused, void *unused2, u32_t what_int, u32_t unused3, BLASTK_thread_context *me)
{
}


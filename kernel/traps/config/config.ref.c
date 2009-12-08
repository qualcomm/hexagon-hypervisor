/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <config.h>
#include <asm_offsets.h>
#include <thread.h>

typedef void (*configptr_t)(u32_t, void *, u32_t, u32_t, H2K_thread_context *);

#define MAX_CONFIGS 2

static const configptr_t H2K_configtab[MAX_CONFIGS] = {
	H2K_trap_config_addthreads,
	H2K_trap_config_schedint,
};

void H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)
{
	if (configtype >= MAX_CONFIGS) return;
	H2K_configtab[configtype](0,ptr,val2,val3,me);
}

void H2K_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me)
{
	u32_t ptrtmp = (unsigned int)ptr;
	H2K_thread_context *thread;
	u32_t delta;
	u32_t i;
	if (ptrtmp & (H2K_CONTEXT_ALIGN-1)) {
		delta = ((ptrtmp + (H2K_CONTEXT_ALIGN-1)) & (-H2K_CONTEXT_ALIGN)) - ptrtmp;
		ptrtmp += delta;
		size -= delta;
	}
	for (i = 0; i < size; i += CONTEXT_SIZE) {
		thread = (H2K_thread_context *)(ptrtmp+i);
		H2K_thread_context_clear(thread);
		thread->next = H2K_free_threads;
		H2K_free_threads = thread;
	}
}

void H2K_trap_config_schedint(u32_t unused, void *unused2, u32_t what_int, u32_t unused3, H2K_thread_context *me)
{
}


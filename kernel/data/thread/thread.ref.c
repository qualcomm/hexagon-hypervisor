/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <thread.h>

BLASTK_thread_context *BLASTK_free_threads;
BLASTK_thread_context BLASTK_idle_context;
BLASTK_thread_context BLASTK_boot_context;

void BLASTK_thread_context_clear(BLASTK_thread_context *thread)
{
	u32_t i;
	u64_t *x = (u64_t *)thread;
	for (i = 0; i < (sizeof(*thread)/sizeof(*x)); i++) {
		x[i] = 0;
	}
}


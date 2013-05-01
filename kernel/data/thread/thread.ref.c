/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <thread.h>
#include <max.h>
#include <globals.h>

void H2K_thread_context_clear(H2K_thread_context *thread)
{
	u32_t i;
	u64_t *x = (u64_t *)thread;
	u64_t vmblock_id = thread->vmblock_id;
	for (i = 0; i < (sizeof(*thread)/sizeof(*x)); i++) {
		x[i] = 0;
	}
	thread->vmblock_id = vmblock_id;
}

void H2K_ext_context_clear(H2K_ext_context *ext)
{
	u32_t i;
	u64_t *x = (u64_t *)ext;
	for (i = 0; i < (sizeof(*ext)/sizeof(*x)); i++) {
		x[i] = 0;
	}
}

void H2K_thread_init()
{
}


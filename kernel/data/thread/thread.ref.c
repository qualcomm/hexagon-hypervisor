/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <thread.h>
#include <max.h>

H2K_thread_context *H2K_free_threads;
#if __QDSP6_ARCH__ == 2
H2K_thread_context H2K_idle_context;
#endif
H2K_thread_context H2K_boot_context;

void H2K_thread_context_clear(H2K_thread_context *thread)
{
	u32_t i;
	u64_t *x = (u64_t *)thread;
	for (i = 0; i < (sizeof(*thread)/sizeof(*x)); i++) {
		x[i] = 0;
	}
}

void H2K_thread_init()
{
	H2K_thread_context_clear(&H2K_boot_context);
#if __QDSP6_ARCH__ == 2
	H2K_thread_context_clear(&H2K_idle_context);
#endif
	H2K_free_threads = NULL;
}


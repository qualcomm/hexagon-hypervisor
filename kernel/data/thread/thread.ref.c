/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <thread.h>
#include <max.h>

BLASTK_thread_context *BLASTK_free_threads;
#if __QDSP6_ARCH__ == 2
BLASTK_thread_context BLASTK_idle_context;
#endif
BLASTK_thread_context BLASTK_boot_context;

BLASTK_fastint_context BLASTK_fastint_contexts[MAX_HTHREADS];

void BLASTK_thread_context_clear(BLASTK_thread_context *thread)
{
	u32_t i;
	u64_t *x = (u64_t *)thread;
	for (i = 0; i < (sizeof(*thread)/sizeof(*x)); i++) {
		x[i] = 0;
	}
}

void BLASTK_thread_init()
{
	int i;
	BLASTK_thread_context_clear(&BLASTK_boot_context);
#if __QDSP6_ARCH__ == 2
	BLASTK_thread_context_clear(&BLASTK_idle_context);
#endif
	BLASTK_free_threads = NULL;
	for (i = 0; i < MAX_HTHREADS; i++) {
		BLASTK_thread_context_clear(&BLASTK_fastint_contexts[i].context);
	}
}


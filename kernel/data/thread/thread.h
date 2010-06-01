/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef THREAD_H
#define THREAD_H 1

#include <context.h>

// extern H2K_thread_context *H2K_free_threads IN_SECTION(".data.thread.freeptr");
#if __QDSP6_ARCH__ == 2
extern H2K_thread_context H2K_idle_context IN_SECTION(".data.core.idle");
#endif
extern H2K_thread_context H2K_boot_context IN_SECTION(".data.init.bootthread");

void H2K_thread_init() IN_SECTION(".text.init.thread");
void H2K_thread_context_clear(H2K_thread_context *thread) IN_SECTION(".text.misc.thread");

#endif


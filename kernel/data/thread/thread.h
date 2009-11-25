/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef THREAD_H
#define THREAD_H 1

#include <context.h>

extern BLASTK_thread_context *BLASTK_free_threads IN_SECTION(".data.thread.free");
extern BLASTK_thread_context BLASTK_idle_context IN_SECTION(".data.thread.idle");
extern BLASTK_thread_context BLASTK_boot_context IN_SECTION(".data.thread.boot");

void BLASTK_thread_context_clear(BLASTK_thread_context *thread);

#endif


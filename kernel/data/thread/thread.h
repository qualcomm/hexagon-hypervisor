/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef THREAD_H
#define THREAD_H 1

#include <context.h>

extern BLASTK_thread_context *BLASTK_free_threads;
extern BLASTK_thread_context BLASTK_idle_context;
extern BLASTK_thread_context BLASTK_boot_context;

void BLASTK_thread_context_clear(BLASTK_thread_context *thread);

#endif


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_FATAL_H
#define BLAST_FATAL_H 1

#include <context.h>

void BLASTK_fatal_kernel(s16_t error_id, BLASTK_thread_context *me, u32_t info0, u32_t info1, u32_t hthread);
void BLASTK_fatal_thread(s16_t error_id, BLASTK_thread_context *me, u32_t info0, u32_t info1, u32_t hthread);

#endif


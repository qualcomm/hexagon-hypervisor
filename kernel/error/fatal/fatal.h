/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_FATAL_H
#define BLAST_FATAL_H 1

#include <context.h>

void BLASTK_fatal_kernel(short error_id, BLASTK_thread_context *me, int info0, int info1, int hthread);
void BLASTK_fatal_thread(short error_id, BLASTK_thread_context *me, int info0, int info1, int hthread);

#endif


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_TID_H
#define BLASTK_TID_H 1

#include <c_std.h>
#include <context.h>

void BLASTK_tid_set(u32_t tid, BLASTK_thread_context *me);
u32_t BLASTK_tid_get(BLASTK_thread_context *me);

#endif


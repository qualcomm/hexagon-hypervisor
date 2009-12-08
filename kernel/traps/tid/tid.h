/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TID_H
#define H2K_TID_H 1

#include <c_std.h>
#include <context.h>

void H2K_tid_set(u32_t tid, H2K_thread_context *me);
u32_t H2K_tid_get(H2K_thread_context *me);

#endif


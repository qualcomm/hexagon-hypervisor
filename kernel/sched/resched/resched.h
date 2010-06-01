/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_RESCHED_H
#define H2K_RESCHED_H 1

#include <c_std.h>
#include <context.h>

void H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum) IN_SECTION(".text.core.resched");

#endif


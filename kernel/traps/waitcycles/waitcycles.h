/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_WAITCYCLES_H
#define H2K_WAITCYCLES_H 1

#include <c_std.h>
#include <context.h>

u64_t H2K_waitcycles_get(u32_t htid, H2K_thread_context *me) IN_SECTION(".text.misc.waitcycles");

#endif


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PRIO_H
#define H2K_PRIO_H 1

#include <c_std.h>
#include <context.h>

s32_t H2K_prio_set(H2K_thread_context *dest, u32_t prio, H2K_thread_context *me) IN_SECTION(".text.misc.prio");
u32_t H2K_prio_get(unsigned int threadid_in, H2K_thread_context *me) IN_SECTION(".text.misc.prio");

#endif


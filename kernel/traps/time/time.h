/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TIME_H
#define H2K_TIME_H 1

#include <c_std.h>
#include <context.h>

u64_t H2K_cputime_get(H2K_thread_context *me) IN_SECTION(".text.misc.cpuime");

u64_t H2K_pcycles_get(H2K_thread_context *me) IN_SECTION(".text.misc.cpuime");

#endif


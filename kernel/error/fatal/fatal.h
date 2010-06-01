/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FATAL_H
#define H2K_FATAL_H 1

#include <context.h>

extern void (*H2K_fatal_kernel_handler)(u32_t) __attribute__((noreturn)) IN_SECTION(".data.misc.fatal");

void H2K_fatal_kernel(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread) IN_SECTION(".text.misc.fatal");
void H2K_fatal_thread(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread) IN_SECTION(".text.misc.fatal");
void H2K_fatal_init() IN_SECTION(".text.init.fatal");

#endif


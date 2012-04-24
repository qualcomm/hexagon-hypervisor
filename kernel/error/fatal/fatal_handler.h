/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FATAL_HANDLER_H
#define H2K_FATAL_HANDLER_H 1

extern void (*H2K_fatal_kernel_handler)(u32_t) __attribute__((noreturn)) IN_SECTION(".data.misc.fatal");

#endif

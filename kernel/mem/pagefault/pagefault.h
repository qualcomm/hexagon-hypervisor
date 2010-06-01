/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PAGEFAULT_H
#define H2K_PAGEFAULT_H 1

#include <c_std.h>
#include <context.h>
void H2K_mem_pagefault(u32_t va, H2K_thread_context *me) IN_SECTION(".text.mem.pagefault");

#endif


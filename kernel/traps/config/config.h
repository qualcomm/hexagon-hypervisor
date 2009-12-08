/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CONFIG_H
#define H2K_CONFIG_H 1

#include <c_std.h>
#include <context.h>

void H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me);
void H2K_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me);
void H2K_trap_config_schedint(u32_t unused, void *unused2, u32_t what_int, u32_t unused3, H2K_thread_context *me);

#endif


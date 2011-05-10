/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PMU_H
#define H2K_PMU_H 1

#include <c_std.h>
#include <context.h>

u32_t H2K_trap_pmuconfig(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_pmuconfig_threadset(u32_t unused, H2K_thread_context *dest, u32_t turnon, u32_t unused2, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_pmuconfig_setreg(u32_t unused, void *handler, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_pmuconfig_getreg(u32_t unused, void *handler, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.config");

#endif


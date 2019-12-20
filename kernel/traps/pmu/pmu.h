/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PMU_H
#define H2K_PMU_H 1

#include <c_std.h>
#include <context.h>
#include <h2_common_pmu.h>

u32_t H2K_trap_pmuctrl(pmuop_type configtype, u32_t val1, u32_t val2, u32_t val3, H2K_thread_context *me) IN_SECTION(".text.config.config");

u32_t H2K_trap_pmuctrl_setreg(u32_t unused, u32_t unused1, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_pmuctrl_getreg(u32_t unused, u32_t unused1, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.config");

#endif


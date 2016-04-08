/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_INTCONFIG_H
#define H2K_INTCONFIG_H 1

#include <c_std.h>
#include <context.h>
#include <max.h>

extern H2K_fastint_context H2K_fastint_contexts[] IN_SECTION(".data.core.interrupt");

void H2K_register_fastint(u32_t whatint, int (*fastint_handler)(u32_t x), H2K_thread_context *me) IN_SECTION(".text.misc.fastint");
void H2K_register_passthru(u32_t phys_int, H2K_id_t id, u32_t virt_int);
void H2K_intconfig_init(u32_t ssbase) IN_SECTION(".text.init.int");

#ifdef CRASH_DEBUG
void H2K_intconfig_l2vic_crash() IN_SECTION(".text.error.crash");
#endif

#endif


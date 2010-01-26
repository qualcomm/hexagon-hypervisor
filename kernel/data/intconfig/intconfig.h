/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_INTCONFIG_H
#define H2K_INTCONFIG_H 1

#include <c_std.h>
#include <context.h>
#include <max.h>

//extern void *H2K_fastint_funcptrs[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
//extern void *H2K_inthandlers[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
//extern u32_t H2K_fastint_mask IN_SECTION(".data.event.interrupt");
extern H2K_fastint_context H2K_fastint_contexts[] IN_SECTION(".data.event.interrupt");
//extern u32_t H2K_fastint_gp IN_SECTION(".data.event.interrupt");

void H2K_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), H2K_thread_context *me);
void H2K_intconfig_init();

#endif


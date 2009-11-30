/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_INTCONFIG_H
#define BLASTK_INTCONFIG_H 1

#include <c_std.h>
#include <context.h>
#include <max.h>

extern void *BLASTK_fastint_funcptrs[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
extern void *BLASTK_inthandlers[MAX_INTERRUPTS] IN_SECTION(".data.event.interrupt");
extern u32_t BLASTK_fastint_mask IN_SECTION(".data.event.interrupt");

void BLASTK_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), BLASTK_thread_context *me);
void BLASTK_intconfig_init();

#endif


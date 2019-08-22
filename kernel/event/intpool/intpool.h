/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_INTPOOL_H
#define H2K_INTPOOL_H 1

#include <context.h>
#include <vmblock.h>

void H2K_intpool_int(u32_t intnum, H2K_thread_context *me, u32_t hwtnum, H2K_vmblock_t *param);
int H2K_intpool_wait(u32_t int_ack_no, H2K_thread_context *me);
void H2K_intpool_cancel(H2K_thread_context *dst);
int H2K_intpool_configure(u32_t intno, u32_t enable, H2K_thread_context *me);

#endif


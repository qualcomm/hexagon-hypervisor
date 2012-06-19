/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_POPUP_H
#define H2K_POPUP_H 1

#include <context.h>

void H2K_popup_int(u32_t intnum, H2K_thread_context *me, u32_t hwtnum, H2K_thread_context *param);
int H2K_popup_wait(u32_t intno, H2K_thread_context *me);
void H2K_popup_cancel(H2K_thread_context *dst);

#endif


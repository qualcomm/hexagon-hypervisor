/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_PRIO_H
#define BLASTK_PRIO_H 1

#include <c_std.h>
#include <context.h>

u32_t BLASTK_prio_set(BLASTK_thread_context *dest, u32_t prio, BLASTK_thread_context *me);
u32_t BLASTK_prio_get(BLASTK_thread_context *me);

#endif


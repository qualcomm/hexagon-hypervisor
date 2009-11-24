/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_RESCHED_H
#define BLASTK_RESCHED_H 1

#include <c_std.h>
#include <context.h>

void BLASTK_reschedule_from_wait(u32_t hwtnum);
void BLASTK_reschedule_from_lowprio(u32_t unused, BLASTK_thread_context *me, u32_t hwtnum);

#endif


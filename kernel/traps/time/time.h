/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_TIME_H
#define BLASTK_TIME_H 1

#include <c_std.h>
#include <context.h>

u64_t BLASTK_cputime_get(BLASTK_thread_context *me);

u64_t BLASTK_pcycles_get(BLASTK_thread_context *me);

#endif


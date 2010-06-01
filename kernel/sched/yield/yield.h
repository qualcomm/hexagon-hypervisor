/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_YIELD_H
#define H2K_YIELD_H 1

void H2K_sched_yield(H2K_thread_context *me) IN_SECTION(".text.core.yield");

#endif


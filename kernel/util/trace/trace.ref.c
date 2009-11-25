/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <trace.h>

u32_t BLASTK_trace_index IN_SECTION(".data.trace") = 0;
BLASTK_trace_entry_t BLASTK_trace_buf[MAX_TRACE_ENTRIES] IN_SECTION(".data.trace");


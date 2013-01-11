/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TRACE_H
#define TRACE_H 1

#include <c_std.h>
#include <q6protos.h>
#include <trace_constants.h>

typedef union {
	u64_t raw;
	struct {
		u64_t htid:4;
		u64_t cycle:28;
		u64_t what:8;
		u64_t info:24;
	};
} H2K_trace_entry_t;

// extern H2K_trace_info_t H2K_trace_info IN_SECTION(".data.trace");

void H2K_trace(s8_t type, u32_t info, u32_t pcyclelo, u8_t hwtnum) IN_SECTION(".text.misc.trace");

void H2K_trace_init() IN_SECTION(".text.init.trace");

#endif


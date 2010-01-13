/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TRACE_H
#define TRACE_H 1

#define MAX_TRACE_ENTRIES 64

#include <c_std.h>
#include <q6protos.h>
#include <trace_constants.h>

typedef union {
	u32_t raw;
	struct {
		s16_t delta;
		u8_t tid;
		u8_t hwtnum:4;
		s8_t id:4;
	};
} H2K_trace_entry_t;

typedef struct {
	H2K_trace_entry_t *buf;
	u32_t entries;
	u32_t index;
	u32_t last_pcyclelo;
	s32_t max_trace_level;
} H2K_trace_info_t;

extern H2K_trace_info_t H2K_trace_info IN_SECTION(".data.trace");

void H2K_trace(s8_t type, u8_t hwtnum, u8_t tid, u32_t pcyclelo);
void H2K_trace_init();

#endif


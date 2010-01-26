/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <trace.h>
#include <max.h>
#include <q6protos.h>
#include <globals.h>

H2K_trace_entry_t H2K_trace_buf_default[MAX_TRACE_ENTRIES] IN_SECTION(".data.trace");

// H2K_trace_info_t H2K_trace_info IN_SECTION(".data.trace") __attribute__((aligned(32)));

void H2K_trace(s8_t type, u8_t hwtnum, u8_t tid, u32_t pcyclelo)
{
	s32_t diff;
	H2K_trace_entry_t val;

	if (type > H2K_gp->trace_info.max_trace_level) return;

	diff = pcyclelo - H2K_gp->trace_info.last_pcyclelo;
	val.delta = Q6_R_sath_R(diff);
	val.id = type;
	val.hwtnum = hwtnum;
	val.tid = tid;

	H2K_gp->trace_info.last_pcyclelo = pcyclelo;

	H2K_gp->trace_info.buf[H2K_gp->trace_info.index++] = val;
	if (H2K_gp->trace_info.index >= H2K_gp->trace_info.entries) H2K_gp->trace_info.index = 0;
}

void H2K_trace_init()
{
	H2K_gp->trace_info.buf = H2K_trace_buf_default;
	H2K_gp->trace_info.entries = MAX_TRACE_ENTRIES;
	H2K_gp->trace_info.index = 0;
	H2K_gp->trace_info.last_pcyclelo = 0;
	H2K_gp->trace_info.max_trace_level = MAX_TRACE_LEVEL;
}


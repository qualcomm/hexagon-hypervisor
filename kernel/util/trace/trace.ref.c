/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <trace.h>
#include <max.h>
#include <q6protos.h>
#include <hw.h>
#include <id.h>
#include <atomic.h>

/* Or, move to end of Globals */

void H2K_trace(s8_t type, u32_t info, u32_t pcyclelo, u8_t hwtnum)
{
	u32_t oldidx,newidx;
	H2K_trace_entry_t val;
	val.cycle = pcyclelo >> 4;
	val.htid = hwtnum;
	val.what = type;
	val.info = info;
	do {
		oldidx = H2K_gp->trace_info_index;
		newidx = oldidx + 1;
		if (newidx >= H2K_gp->trace_info_entries) newidx = 0;
	} while (H2K_atomic_compare_swap(&H2K_gp->trace_info_index,oldidx,newidx) != oldidx);
	H2K_gp->trace_info_buf[oldidx] = val.raw;
	H2K_dccleana(&H2K_gp->trace_info_buf[oldidx]);
}

void H2K_trace_init()
{
	H2K_gp->trace_info_buf = H2K_gp->trace_buf_default;
	H2K_gp->trace_info_entries = DEFAULT_TRACE_ENTRIES;
	H2K_gp->trace_info_index = 0;
}


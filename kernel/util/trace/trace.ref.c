/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <trace.h>
#include <max.h>
#include <hexagon_protos.h>
#include <hw.h>
#include <id.h>
#include <atomic.h>

/* Or, move to end of Globals */

void H2K_trace(s8_t type, u32_t info, u32_t pcyclelo, u8_t hwtnum)
{
	H2K_trace_entry_t entry;
	entry.what = type;
	entry.info = info;
	H2K_hw_trace(entry.raw);
}

void H2K_trace_init()
{
}


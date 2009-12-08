/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TRACE_H
#define TRACE_H 1

#define MAX_TRACE_ENTRIES 16

#include <c_std.h>
#include <q6protos.h>

typedef struct {
	u32_t word0;
	u32_t word1;
	u32_t word2;
	u32_t word3;
} __attribute__((aligned(16))) H2K_trace_entry_t;

extern u32_t H2K_trace_index IN_SECTION(".data.trace");
extern H2K_trace_entry_t H2K_trace_buf[] IN_SECTION(".data.trace");

static inline void H2K_trace(s16_t type, u32_t arg1, u32_t arg2, u32_t arg3, u32_t hwtnum)
{
	/* EJP: Fixme: non-atomic update of trace_index */
	u32_t word0;
	u32_t oldidx = H2K_trace_index;
	H2K_trace_index = oldidx + 1;
	if (H2K_trace_index >= MAX_TRACE_ENTRIES) H2K_trace_index = 0;
	word0 = Q6_R_combine_RlRl(hwtnum,type);
	H2K_trace_buf[oldidx].word0 = word0;
	H2K_trace_buf[oldidx].word1 = arg1;
	H2K_trace_buf[oldidx].word2 = arg2;
	H2K_trace_buf[oldidx].word3 = arg3;
}

#endif


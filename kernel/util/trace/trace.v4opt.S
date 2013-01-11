/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <trace_asm_macro.h>
#include <max.h>

// H2K_trace(s8_t type, u32_t info, u32_t pcyclelo, u8_t hwtnum) 
FUNC_START H2K_trace  .text.misc.trace  .p2align  3
	r0 = insert(r1,#24,#8)
	ASM_INLINE_TRACE(r0,r3:2,r3,r2,r5:4,r5,r4,p0,r6,r7,r8,,)
	dccleana(r4)
	jumpr r31
FUNC_END H2K_trace

FUNC_START H2K_trace_init .text.init.trace
	{
		r0 = add(H2K_GP,#KG_trace_buf_default)
		r1 = #DEFAULT_TRACE_ENTRIES
		r2 = #0
	}
	{
		memd(H2K_GP+#KG_trace_info_entries_buf) = r1:0
		memw(H2K_GP+#KG_trace_info_index) = r2
		jumpr r31
	}
FUNC_END H2K_trace_init


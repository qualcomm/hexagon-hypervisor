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
	trace(r0)
	jumpr r31
FUNC_END H2K_trace

FUNC_START H2K_trace_init .text.init.trace
	jumpr r31
FUNC_END H2K_trace_init


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_preempt .text.core.interrupt .p2align 2
	r0 = #1
	crswap(r0,sgp)
	p0 = cmp.eq(r0,#0)	// task was NULL, better to just note that
	if (p0) jump .Lpreempt_done
	r2 = ssr
	r1 = #0			// EX=0 UM=0.. don't change IE
	r3 = r2
	r2 = insert(r1,#2,#16)
	ssr = r2
	isync
	nop
	ssr = r3
	// interrupt was not taken
.Lpreempt_done:
	r3:2 = memd(r0+#CONTEXT_r0100)
	crswap(r0,sgp)		// swap task back in
	r1:0 = r3:2
	jumpr r31		// just return


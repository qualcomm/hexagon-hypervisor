/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_preempt .text.core.interrupt .falign
	{
		r1:0 = combine(#0,#1)	// EX=0 UM=0
		r4 = elr
	}
	{
		crswap(r0,sgp0)
		if (cmp.eq(r0.new,#0)) jump:nt .Lpreempt_done
	}
	r2 = ssr
	{
		r2 = insert(r1,#2,#16)
		r3 = r2
		memw(r0+#CONTEXT_elr) = r4
	}
	{
		ssr = r2
	}
	isync
	ssr = r3
	// interrupt was not taken
.Lpreempt_done:
	sgp0 = r0			// swap task back in
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) r1:0 = memd(r0+#CONTEXT_r0100)
		jumpr r31		// just return
	}


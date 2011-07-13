/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

FUNC_START H2K_preempt .text.core.interrupt .falign
	{
		r1:0 = combine(#4,#1)	// IE=1 EX=0 UM=0
		r2 = ssr
	}
	{
		crswap(r0,sgp)
		if (cmp.eq(r0.new,#0)) jump:nt .Lpreempt_done
		r3 = r2
		r2 = insert(r1,#3,#16)
	}
	ssr = r2
	isync
	ssr = r3
	// interrupt was not taken
.Lpreempt_done:
	sgp = r0			// swap task back in
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) r1:0 = memd(r0+#CONTEXT_r0100)
		jumpr r31		// just return
	}


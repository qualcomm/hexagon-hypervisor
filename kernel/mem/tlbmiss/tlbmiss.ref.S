/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/* Don't fear! */

/* 
 * We do a lot of context save and restore in this reference version,
 * to support handlers written in C.
 */

/*
 * The optimized handlers will be in assembly and we can use
 * custom calling conventions and such.
 */

/* 
 * However, in both cases there is one invariant: we need to
 * save any context that might be modified, just like an interrupt
 */

/*
 * NOTE: tlb is locked!
 */

/* Only need to save caller-save regs */
FUNC_START H2K_handle_tlbmissx .text.mem.tlbmiss .p2align 2
1:
	// jump 1b
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* r7 = me, r8 = ssr, r9 = htid */
	r3 = and(r8,#3)
	r2 = elr
	p0 = cmp.eq(r3,#1)
	p1 = cmp.eq(r3,#2)
	if (p0) r2 = add(r2,#16)
	if (!p1) jump .Ltlbmiss_common
	r2 = badva
	jump .Ltlbmiss_common

FUNC_END H2K_handle_tlbmissx

FUNC_START H2K_handle_tlbmissrw .text.mem.tlbmiss .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* r7 = me, r8 = ssr, r9 = htid */
	r2 = badva
.Ltlbmiss_common:
	r1:0 = combine(r7,r2)	// H2K_mem_tlb_fill(va,me)
	call H2K_mem_tlb_fill // unlocks
	jump H2K_context_restore_return

FUNC_END H2K_handle_tlbmissrw


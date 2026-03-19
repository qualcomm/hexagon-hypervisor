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

#ifdef COUNT_TLB_EVENTS
	// atomically increment me->vmblock->tlbmissx
	r10 = memw(r7 + #CONTEXT_vmblock)
	r10 = add(r10, #VMBLOCK_tlbmissx)
	r5:4 = #1
2:
	r3:2 = memd_locked(r10)
	r3:2 = add(r3:2, r5:4)
	memd_locked(r10, p0) = r3:2
	if !p0 jump 2b
3:
#endif

	r3 = and(r8,#3)
	r2 = elr
	p0 = cmp.eq(r3,#1)
	p1 = cmp.eq(r3,#2)
	if (p0) r2 = add(r2,#16)
	if (!p1) jump H2K_tlbmiss_common
	r2 = badva
	jump H2K_tlbmiss_common

FUNC_END H2K_handle_tlbmissx

FUNC_START H2K_handle_tlbmissrw .text.mem.tlbmiss .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* r7 = me, r8 = ssr, r9 = htid */

#ifdef COUNT_TLB_EVENTS
	// atomically increment me->vmblock->tlbmissrw
	r10 = memw(r7 + #CONTEXT_vmblock)
	r10 = add(r10, #VMBLOCK_tlbmissrw)
	r5:4 = #1
4:
	r3:2 = memd_locked(r10)
	r3:2 = add(r3:2, r5:4)
	memd_locked(r10, p0) = r3:2
	if !p0 jump 4b
5:
#endif

	r2 = badva
.global H2K_tlbmiss_common
H2K_tlbmiss_common:
	r1:0 = combine(r7,r2)	// H2K_mem_tlb_fill(va,me)
	call H2K_mem_tlb_fill // unlocks
	jump H2K_context_restore_return

FUNC_END H2K_handle_tlbmissrw


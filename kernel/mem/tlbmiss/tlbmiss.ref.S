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
FUNC_START H2K_handle_tlbmissx .text.tlbmiss .p2align 2
1:
	// jump 1b
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r0302) = r3:2
	memd(r0+#CONTEXT_r0504) = r5:4
	r5 = ssr
	r3 = p3:0
	r2 = and(r5,#3)
	r4 = elr
	p0 = cmp.eq(r2,#1)
	p1 = cmp.eq(r2,#2)
	if (p0) r2 = add(r4,#16)
	if (!p0) r2 = r4
	if (!p1) jump .Ltlbmiss_common
	r2 = badva
	jump .Ltlbmiss_common

FUNC_END H2K_handle_tlbmissx

FUNC_START H2K_handle_tlbmissrw .text.tlbmiss .p2align 2
	//jump 1b
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r0302) = r3:2
	memd(r0+#CONTEXT_r0504) = r5:4
	r5 = ssr
	r4 = elr
	r3 = p3:0
	r2 = badva
.Ltlbmiss_common:
	memd(r0+#CONTEXT_r0706) = r7:6
	memd(r0+#CONTEXT_r0908) = r9:8
	memd(r0+#CONTEXT_r1110) = r11:10
	memd(r0+#CONTEXT_r1312) = r13:12
	memd(r0+#CONTEXT_r1514) = r15:14
	memd(r0+#CONTEXT_ssrelr) = r5:4

	r15 = r0
	crswap(r0,sgp)
	memd(r15+#CONTEXT_r0100) = r1:0
	r0 = r15
	r4 = r3
	r5 = usr
	r6 = sa0
	r7 = lc0
	r8 = sa1
	r9 = lc1
	r10 = m0
	r11 = m1
	memd(r0+#CONTEXT_sr_preds) = r5:4
	memd(r0+#CONTEXT_lc0sa0) = r7:6
	memd(r0+#CONTEXT_lc1sa1) = r9:8
	memd(r0+#CONTEXT_m1m0) = r11:10
	memd(r0+#CONTEXT_r2928) = r29:28
	memd(r0+#CONTEXT_r3130) = r31:30

	/* Set up stack */

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r3 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r3)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif
	/* Allocate frame */
	allocframe(#8)
	/* Save r17:16 on stack */
	memd(r29+#0) = r17:16
	/* Set up H2K_gp in r16 */
	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)
	/* call TLB miss C */
	r1:0 = combine(r0,r2)	// H2K_mem_tlb_fill(va,me)
	call H2K_mem_tlb_fill
	/* Restore r17:16 */
	r17:16 = memd(r29+#0)
	/* deallocframe */
	deallocframe
	/* Restore r29:28 */
	r0 = sgp
	r29:28 = memd(r0+#CONTEXT_r2928)

	/* Restore control regs */
	r31:30 = memd(r0+#CONTEXT_r3130)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r11:10 = memd(r0+#CONTEXT_m1m0)
	r9:8 = memd(r0+#CONTEXT_lc1sa1)
	r7:6 = memd(r0+#CONTEXT_lc0sa0)
	r5:4 = memd(r0+#CONTEXT_sr_preds)
	r3:2 = memd(r0+#CONTEXT_ssrelr)
	m1 = r11
	m0 = r10
	lc1 = r9
	sa1 = r8
	lc0 = r7
	sa0 = r6
	usr = r5
	p3:0 = r4
	ssr = r3
	elr = r2

	/* Restore user regs */

	r15:14 = memd(r0+#CONTEXT_r1514)
	r13:12 = memd(r0+#CONTEXT_r1312)
	r11:10 = memd(r0+#CONTEXT_r1110)
	r9:8 = memd(r0+#CONTEXT_r0908)
	r7:6 = memd(r0+#CONTEXT_r0706)
	r5:4 = memd(r0+#CONTEXT_r0504)
	r3:2 = memd(r0+#CONTEXT_r0302)
	r1:0 = memd(r0+#CONTEXT_r0100)

	/* Return! And unlock the TLB! */
	rteunlock

FUNC_END H2K_handle_tlbmissrw


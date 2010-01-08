/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

#define SSR_GUEST_BIT 13
#define ERROR_GEVB_OFFSET 0

FUNC_START H2K_handle_error .text.error .p2align 2
	/* TBD: do double exception check as spec'd */
	/* 
	 * XXX: this is rather V3 oriented, we should probably macro 
	 * the guest regs stuff to use guest regs in V4 
	 */
	crswap(r24,sgp)
	memd(r24+#CONTEXT_r1514) = r15:14
	memd(r24+#CONTEXT_r1312) = r13:12
	r12 = memw(r24+#CONTEXT_gevb)
	if (r12 == #0) jump:t 1f
	r15 = ELR
	r14 = BADVA
	r13 = p3:0
	r12 = add(r12,#ERROR_GEVB_OFFSET)
	elr = r12
	memd(r24+#CONTEXT_gelr_gbadva) = r15:14
	r15 = ssr
	p0 = tstbit(r15,#SSR_GUEST_BIT)
	r15:14 = memd(r24+#CONTEXT_gssr_gosp)
	{
		if (!p0) r14 = r29
		if (!p0) r29 = r14
	}
	/* TBD: set up GSSR in r15 */
	memd(r24+#CONTEXT_gssr_gosp) = r15:14
	p3:0 = r13
	r15:14 = memd(r24+#CONTEXT_r1514)
	r13:12 = memd(r24+#CONTEXT_r1312)
	crswap(r24,sgp)
	rte
1:
	/* Set up KSP and jump to fatal_thread */

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r2 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r2)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif
	r0 = #-3
	r1 = r24
	r3:2 = #0
	r4 = r28
	crswap(r24,sgp)
	jump H2K_fatal_thread

FUNC_END H2K_handle_error

FUNC_START H2K_handle_nmi .text.error .p2align 2
	crswap(r24,sgp)
	jump 1b
FUNC_END H2K_handle_nmi

FUNC_START H2K_handle_rsvd .text.error .p2align 2
	crswap(r24,sgp)
	jump 1b
FUNC_END H2K_handle_rsvd

FUNC_START H2K_handle_trap1 .text.error .p2align 2
	crswap(r24,sgp)
	jump 1b
FUNC_END H2K_handle_trap1


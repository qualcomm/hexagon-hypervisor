/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_handle_error .text.vm.error .p2align 2
	/* TBD: do double exception check as spec'd */
	/* 
	 * XXX: this is rather V3 oriented, we should probably macro 
	 * the guest regs stuff to use guest regs in V4 
	 */
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* r7 = me, r8 = ssr, r9 = htid */

	r0 = badva
	r1 = and(r8,#0xff)
	r2 = #ERROR_GEVB_OFFSET
	r3 = r7
	call H2K_vm_event
	call H2K_preempt
	jump H2K_context_restore_return

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
	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)
	r0 = #-3
	r1 = r24
	r3:2 = #0
	r4 = r28
	crswap(r24,sgp)
	jump H2K_fatal_thread

FUNC_END H2K_handle_error

FUNC_START H2K_handle_nmi .text.vm.error .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	r0 = #-3
	r1 = r24
	r3:2 = #0
	r4 = r28
	jump H2K_fatal_thread
FUNC_END H2K_handle_nmi

FUNC_START H2K_handle_rsvd .text.vm.error .p2align 2
	jump H2K_handle_nmi
FUNC_END H2K_handle_rsvd

FUNC_START H2K_handle_debug .text.vm.error .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	r0 = #0
	r1 = and(r8,#0xff)
	r2 = #DEBUG_GEVB_OFFSET
	r3 = r7
	jump H2K_vm_event
FUNC_END H2K_handle_rsvd


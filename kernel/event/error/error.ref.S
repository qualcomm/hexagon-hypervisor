/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
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
	r3 = r7
	r1 = and(r8, #0xff)

#ifdef DO_EXT_SWITCH
	p0 = cmp.eq(r1, #0x16)  // XE exception?
	if (!p0) jump 1f
	r0 = memw(r7 + #CONTEXT_vmblock)
	r10 = memw(r0 + #VMBLOCK_flags)
	p0 = tstbit(r10, #0)
	if (!p0) jump 1f  // VM using ext?

	jump H2K_ext_context_restore // continues to H2K_context_restore_return
#endif

1:
	r0 = badva
	// r1 = cause
	r2 = #ERROR_GEVB_OFFSET
	// r3 = me
	call H2K_vm_event
	call H2K_preempt
	jump H2K_context_restore_return

	/* EJP: Dead Code! */
	/* Set up KSP and jump to fatal_thread */

//#if ARCHV <= 3
//	r29.h = #hi(H2K_stacks)
//	r29.l = #lo(H2K_stacks)
//	r28 = ssr
//	r28 = extractu(r28,#3,#19)
//	r2 = #KERNEL_STACK_SIZE
//	r29 += mpyi(r28,r2)
//#else
//	r28 = HTID
//	r29 = #KERNEL_STACK_SIZE
//	r29 = add(##H2K_stacks,mpyi(r28,r29))
//#endif
//	r4 = r28
//	r28.h = #hi(H2K_kg)
//	r28.l = #lo(H2K_kg)
//	r0 = #-3
//	r1 = r24
//	r3:2 = #0
//	crswap(r24,sgp)
//	jump H2K_fatal_thread

FUNC_END H2K_handle_error

FUNC_START H2K_handle_nmi .text.vm.error .p2align 2

#ifdef NMI_STOP
	r1=##0xe0f0beef
	stop(r0)
#endif

	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	r0 = #-3
	r1 = r7
	r3:2 = #0
	r4 = r9
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
FUNC_END H2K_handle_debug


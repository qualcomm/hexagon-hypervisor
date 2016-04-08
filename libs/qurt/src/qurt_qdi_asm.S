/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.p2align 5

	.globl	qurt_qdi_qhi3
	.globl	qurt_qdi_qhi4
	.globl	qurt_qdi_qhi5
	.globl	qurt_qdi_qhi6
	.globl	qurt_qdi_qhi7
	.globl	qurt_qdi_qhi8
	.globl	qurt_qdi_qhi9
	.globl	qurt_qdi_qhi10
	.globl	qurt_qdi_qhi11
	.globl	qurt_qdi_qhi12

	.type	qurt_qdi_handle_invoke, @function
qurt_qdi_qhi3:
qurt_qdi_qhi4:
qurt_qdi_qhi5:
qurt_qdi_qhi6:
qurt_qdi_qhi7:
qurt_qdi_qhi8:
qurt_qdi_qhi9:
qurt_qdi_qhi10:
qurt_qdi_qhi11:
qurt_qdi_qhi12:
qurt_qdi_handle_invoke:
	/* Handle is related to pointer.  == pointer? pointer>>2? */
	r1 = asl(r1,#2)
	/* Invoke is word @ ptr */
	r28 = memw(r1)
	jumpr r28
	.size	qurt_qdi_handle_invoke, .-qurt_qdi_handle_invoke

#if 0
	/* EJP: does this have anything to do with QDI? */
	.global __stack_chk_fail
	.global __init_stack_chk_guard
	.global __stack_chk_guard
	.global __stack_chk_LR
	.global __stack_chk_SP

	.text
	.align 4
__stack_chk_fail:
	r0 = add(pc, ##__stack_chk_LR@PCREL)
	memw(r0) = LR
	r0 = add(pc, ##__stack_chk_SP@PCREL)
	memw(r0) = SP
	r0 = #-1
	jump ##qurt_exception_raise_nonfatal

	.align 4
__init_stack_chk_guard:
	/* Dummy function for backward compatibility */
	/* This used to do something, but is no longer needed */
	jumpr r31

#ifdef CONFIG_NO_SMALL_DATA
	.data
#else
	.section .sdata.4.4,"aw",@progbits
#endif
	.align 4
__stack_chk_guard:
	.word 0xDEADDEAD

	.data
	.align 4
__stack_chk_LR:
	.word 1
__stack_chk_SP:
	.word 0
#endif

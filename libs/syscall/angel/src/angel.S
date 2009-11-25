/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.text
	.global angel
	.type angel,@function
	.p2align 2
angel:
	allocframe(#0)
	trap0(#0)
	deallocframe
	jumpr r31
	.size angel,.-angel

	.global _exit
	.type exit,@function
	.p2align 2
_exit:
	r1 = r0
	r2 = r0
	r0 = #0x18
	jump angel
	.size _exit,.-_exit

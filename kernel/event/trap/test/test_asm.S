/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.text
	.global call_trap0
call_trap0:
	r10 = ssr
	r10 = insert(r0,#8,#0)
	ssr = r10
	sgp = r1
	r11.h = #hi(1f)
	r11.l = #lo(1f)
	elr = r11
	/* other stuff here */
	jump H2K_handle_trap0
1:
	jumpr r31

	.data
	.global H2K_stacks
	.p2align 5
stacks_start:
	.space 128
H2K_stacks:
	.space 128
	.space 128
	.space 128
	.space 128
	.space 128

	.global H2K_futex_wait
H2K_futex_wait:
	r0 = #2
	jumpr r31

	.global H2K_futex_resume
H2K_futex_resume:
	r0 = #3
	jumpr r31


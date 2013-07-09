/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.section .start,"awx",@progbits

	.global _start
	.global hexagon_pre_main
_start:
	jump start
hexagon_pre_main:
	r29 = ##stack
	{
		r0 = r29
		r1 = #1
		call main
	}
	jump start

	.data
	.p2align 8
	.space 1024
stack:
	.word stack
	.word 0
	

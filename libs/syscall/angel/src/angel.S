/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.text
	.global __angel
	.type __angel,@function
	.p2align 2
__angel:
	allocframe(#0)
	trap0(#0)
	deallocframe
	jumpr r31
	.size __angel,.-__angel

	.global _exit
	.type exit,@function
	.p2align 2
_exit:
	jump sys_exit
	.size _exit,.-_exit

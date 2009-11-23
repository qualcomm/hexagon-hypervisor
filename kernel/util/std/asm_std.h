/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_ASM_STD_H
#define BLASTK_ASM_STD_H 1

#define NULL 0

.macro FUNC_START name, section, align, amt
	.section \section,"ax",@progbits
	\align \amt
	.global \name
	.type \name,@function
\name:
.endm

.macro FUNC_END name
	.size \name,.-\name
.endm

.macro DATA_START name, section, align, amt
	.section \section,"aw",@progbits
	\align \amt
	.global \name
	.type \name,@object
\name:
.endm

.macro DATA_END name
	.size \name,.-\name
.endm

#endif


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

// TRACE32 Extension Development Kit 
// C run-time initialization for TRACE32 TASK/EXTension
// Copyright Lauterbach GmbH

	.file    "crt0.S"
	.section ".text"

	.align   2
	.globl   _start
	.type    _start, @function
_start:
	b main
	.size    _start, .-_start

	.align   2
	.globl   __eabi
	.type    __eabi, @function
__eabi:
	blr
	.size    __eabi, .-__eabi

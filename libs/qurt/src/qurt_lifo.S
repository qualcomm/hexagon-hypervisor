/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//#include "qurt_asm.h"
	.text
//    ASM_SECTION(.text)
	.global qurt_lifo_pop 
	.global qurt_lifo_push 
	.type	qurt_lifo_pop, @function
qurt_lifo_pop: 
	r2 = memw_locked(r0)
	{
		p0 = cmp.eq(r2,#0)
		if p0.new jump:nt 1f 
		if !p0.new r1 = memw( r2 ) 
	}
	memw_locked(r0,p0) = r1
	{
		if p0 jumpr r31
		if p0 r0 = r2
	}
	jump qurt_lifo_pop
1:
	{
		r0 = r2
		jumpr r31
	}
	.size qurt_lifo_pop, .-qurt_lifo_pop

	.type	qurt_lifo_push, @function
qurt_lifo_push:
	r2 = memw_locked(r0)
    memw(r1) = r2
	memw_locked(r0,p0) = r1
	if (p0) jumpr r31
	jump qurt_lifo_push
	.size qurt_lifo_push, .-qurt_lifo_push


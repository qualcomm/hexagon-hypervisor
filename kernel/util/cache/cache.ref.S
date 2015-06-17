/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

#if ARCHV <= 3
#define L2_SETS (256*1024/32/4/4)
#define L2_WAYS 4
#define DC_SETS (32*1024/32/16)
#define DC_WAYS 16
#define IC_SETS (32*1024/32/8)
#define IC_WAYS 8
#elif ARCHV == 4
#define L2_SETS (256*1024/32/4)
#define L2_WAYS 4
#define DC_SETS (16*1024/32/8)
#define DC_WAYS 8
#define IC_SETS (16*1024/32/8)
#define IC_WAYS 8
#elif ARCHV >= 5
#define L2_WAYS 4
#define L2_SETS (512*1024/32/L2_WAYS)
#define DC_WAYS 8
#define DC_SETS (32*1024/32/DC_WAYS)
#define IC_WAYS 8
#define IC_SETS (32*1024/32/IC_WAYS)
#endif

FUNC_START H2K_cache_l2_cleaninv .text.misc.cache .p2align 4
	r0 = #0
	r1 = #L2_SETS
	loop1(1f,#L2_WAYS)
1:
	loop0(1f,r1)
	.falign
1:
	{
		l2cleaninvidx(r0)
		r0 = add(r0,#0x100)	// set in bits N:8
	}:endloop0
	{
		r0 = add(r0,#1)
	}:endloop1
	jumpr r31
FUNC_END H2K_cache_l2_cleaninv

FUNC_START H2K_cache_d_cleaninv .text.misc.cache .p2align 4
	r0 = #0
	loop1(1f,#DC_WAYS)
1:
	loop0(1f,#DC_SETS)
	.falign
1:
	{
		dccleaninvidx(r0)
		r0 = add(r0,#32)	// set in bits N:5
	}:endloop0
	{
		r0 = add(r0,#1)
	}:endloop1
	jumpr r31
FUNC_END H2K_cache_d_cleaninv

FUNC_START H2K_cache_d_clean .text.misc.cache .p2align 4
	r0 = #0
	loop1(1f,#DC_WAYS)
1:
	loop0(1f,#DC_SETS)
	.falign
1:
	{
		dccleanidx(r0)
		r0 = add(r0,#32)
	}:endloop0
	{
		r0 = add(r0,#1)
	}:endloop1
	jumpr r31
FUNC_END H2K_cache_d_clean

FUNC_START H2K_cache_i_inv .text.misc.cache .p2align 4
	r0 = #0
	loop1(1f,#IC_WAYS)
1:
	loop0(1f,#IC_SETS)
1:
	icinvidx(r0)
	{
		r0 = add(r0,#32)
	}:endloop0
	{
		r0 = add(r0,#1)
	}:endloop1
	jumpr r31
FUNC_END H2K_cache_i_inv


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"

        .text

FUNC_START h2_cache_dcclean_range .text.h2.cache .p2align 4
        // number of 32-byte chunks, rounded up
	{
		p0 = cmp.eq(r1,#0)
		if (p0.new) jumpr:nt r31
		r2 = and(r0,#31)	// low bits to add in
		r0 = and(r0,#-32)
	}
	{
		r1 += add(r2,#31)
	}
        r1 = lsr(r1,#5)
        loop0(1f,r1)
        .falign
1:
        {
                dccleana(r0)
		r0 = add(r0,#32)
        }:endloop0
        jumpr r31
FUNC_END h2_cache_dcclean_range

FUNC_START h2_cache_dccleaninv_range .text.h2.cache .p2align 4
        // number of 32-byte chunks, rounded up
	{
		p0 = cmp.eq(r1,#0)
		if (p0.new) jumpr:nt r31
		r2 = and(r0,#31)	// low bits to add in
		r0 = and(r0,#-32)
	}
	{
		r1 += add(r2,#31)
	}
        r1 = lsr(r1,#5)
        loop0(1f,r1)
        .falign
1:
        {
                dccleaninva(r0)
                r0 = add(r0,#32)
        }:endloop0
        jumpr r31
FUNC_END h2_cache_dccleaninv_range

FUNC_START h2_cache_icinv_range .text.h2.cache .p2align 4
        // number of 32-byte chunks, rounded up
	{
		p0 = cmp.eq(r1,#0)
		if (p0.new) jumpr:nt r31
		r2 = and(r0,#31)	// low bits to add in
		r0 = and(r0,#-32)
	}
	{
		r1 += add(r2,#31)
	}
        r1 = lsr(r1,#5)
        loop0(1f,r1)
        .falign
1:
	icinva(r0)	// solo
        {
                r0 = add(r0,#32)
        }:endloop0
        jumpr r31
FUNC_END h2_cache_icinv_range


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

        .text
	.global h2_cache_dcclean_range
        .global h2_cache_dccleaninv_range
        .global h2_cache_icinv_range

	.p2align 5
h2_cache_dcclean_range:
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

	.p2align 5
h2_cache_dccleaninv_range:
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

	.p2align 5
h2_cache_icinv_range:
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


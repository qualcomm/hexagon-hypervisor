/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

// int qmem_cache_clean(void *data, size_t size, qmem_cache_op_t opcode, qmem_cache_t type);

	.text
	.global qmem_blast_inv_icache
	.global qmem_blast_inv_dcache
	.global qmem_blast_clean_dcache
	.global qmem_blast_cleaninv_dcache
qmem_blast_inv_icache:
	// number of 32-byte chunks, rounded up
	p0 = cmp.eq(r1,#0)
	if (p0) jumpr r31
	r3 = add(r1,#31)
	r3 = lsr(r3,#5)
	loop0(1f,r3)
	.falign
1:
	icinva(r0)
	{
		r0 = add(r0,#32)
	}:endloop0
	jumpr r31

qmem_blast_inv_dcache:
	// number of 32-byte chunks, rounded up
	p0 = cmp.eq(r1,#0)
	if (p0) jumpr r31
	r3 = add(r1,#31)
	r3 = lsr(r3,#5)
	loop0(1f,r3)
	.falign
1:
	dcinva(r0)
	{
		dcinva(r0)		// do it twice for v2- errata
		r0 = add(r0,#32)
	}:endloop0
	jumpr r31

qmem_blast_clean_dcache:
	// number of 32-byte chunks, rounded up
	p0 = cmp.eq(r1,#0)
	if (p0) jumpr r31
	r3 = add(r1,#31)
	r3 = lsr(r3,#5)
	loop0(1f,r3)
	.falign
1:
	{
		dccleana(r0)
		r0 = add(r0,#32)
	}:endloop0
	jumpr r31

qmem_blast_cleaninv_dcache:
	// number of 32-byte chunks, rounded up
	p0 = cmp.eq(r1,#0)
	if (p0) jumpr r31
	r3 = add(r1,#31)
	r3 = lsr(r3,#5)
	loop0(1f,r3)
	.falign
1:
	{
		dccleaninva(r0)
		r0 = add(r0,#32)
	}:endloop0
	jumpr r31

	.data
	.global qmem_default_pool
	.p2align 3
qmem_default_pool:
	.word 0
	.word 0


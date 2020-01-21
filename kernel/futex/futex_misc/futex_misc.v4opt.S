/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <futex.h>
#include <max.h>
#include <safemem.h>

FUNC_START H2K_futex_cancel .text.core.futex .p2align 2 // futex_cancel(dest)
#define DEST_IN r0
#define RING r0
#define DEST r1
#define HASH r3
#define PA r5:4
#define PA_LO r4
#define PA_HI r5
	{
		PA = memd(DEST_IN+#CONTEXT_futex_ptr_64)
		RING = add(H2K_GP,#KG_futexhash)
		DEST = DEST_IN
	}
	{
		HASH =+mpyi(PA_LO,##FUTEX_PRIME)
		PA_HI = and(PA_HI,#3)
	}
	{
		HASH ^= asl(PA_HI,#32-FUTEX_HASHBITS)
	}
	{
		RING = tableidxw(HASH,#FUTEX_HASHBITS,#32-FUTEX_HASHBITS)
		memw(DEST+#CONTEXT_r00) = #-1
		jump H2K_ring_remove_real
	}
#undef HASH
#undef DEST_IN
#undef RING
#undef DEST
#undef PA
FUNC_END H2K_futex_cancel

FUNC_START H2K_futex_init .text.core.futex .p2align 2 // futex_cancel(dest)
#define PTR r0
	{
		loop0(1f,#FUTEX_HASHSIZE)
		PTR = add(H2K_GP,#KG_futexhash)
	}
1:
	{
		memw(PTR) = #0
		PTR = add(PTR,#4)
	}:endloop0
	jumpr r31
#undef PTR
FUNC_END H2K_futex_init


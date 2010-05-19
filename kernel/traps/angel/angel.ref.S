/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

#if 1
FUNC_START H2K_trap_angel .text.event.trap0 .p2align 2
        jumpr r31
        .size BLASK_trap_angel, .-H2K_trap_angel
FUNC_END H2K_trap_angel
#else

FUNC_START H2K_trap_angel .text.trap.angel .p2align 2
	r6 = #0x20
#if __QDSP6_ARCH__ <= 3
	r7 = ssr
	r7 = extractu(r7,#3,#19)
#else
	r7 = htid
#endif
	r8 = #1
	k0lock
	memd(r6+#0) = r1:0
	memw(r6+#8) = r7
	dccleaninva(r6)
	r5:4 = memd(r6+#0)
	p0 = cmp.eq(r5:4,r1:0)
	if (!p0) jump 9f
	memw(r6+#12) = r8
	dccleaninva(r6)
	r8 = memub(r6)
	dccleaninva(r6)
	loop0(1f,#66)
	.falign
1:
	{ nop }:endloop0
	r8 = memw(r6+#12)
	dccleaninva(r6)
	r1:0 = memd(r6+#0)
	k0unlock
	jumpr r31
9:
	r5 = #0x1c
	r4.h = #0xdead
	r4.l = #0xbeef
	memw(r5) = r4
	jump 9b

FUNC_END H2K_trap_angel

#endif


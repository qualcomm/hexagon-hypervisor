/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>
#include <h2_common_pmap.h>

#ifdef NULL_ANGEL_TRAP
FUNC_START H2K_trap_angel .text.event.trap0 .p2align 2
        jumpr r31
        .size BLASK_trap_angel, .-H2K_trap_angel
FUNC_END H2K_trap_angel

#else
FUNC_START H2K_trap_angel .text.trap.angel .p2align 2

	// save regs clobbered by calls
	r12 = r31
	r11:10 = r1:0

	r0 = add(H2K_GP, #KG_core_id)
	r0 = memw(r0)
	r0 = add(r0, #1)
	r6 = mpyi(r0, #0x20)
	r6 = add(r6, #ANGEL_VA)

	r0 = add(H2K_GP, #KG_angel_lock)
	call H2K_spinlock_lock

	r1:0 = r11:10
	r2 = htid
	r8 = #1
	memd(r6+#0) = r1:0
	memw(r6+#8) = r2
	{
		p0 = cmp.eq(r0,#0x18)	// is it exit?
		if (p0.new) memw(r6+#8) = r1  // if so, we should have kept around r2 but this is probably OK
		if (p0.new) r2 = r1
	}
	dccleaninva(r6)
	r5:4 = memd(r6+#0)
	p0 = cmp.eq(r5:4,r1:0)
	if (!p0) jump .Lerror

	memw(r6+#12) = r8
	dccleaninva(r6)
	r8 = memub(r6+#15)
	dccleaninva(r6)

	loop0(.Lwait,#66)
	.falign
.Lwait:
	{ nop }:endloop0

	r8 = memw(r6+#12)
	dccleaninva(r6)
	r11:10 = memd(r6+#0)

	r0 = add(H2K_GP, #KG_angel_lock)
	call H2K_spinlock_unlock

	r1:0 = r11:10
	r31 = r12  // restore
	jumpr r31
.Lerror:
	r5 = #0x1c
	r4.h = #0xdead
	r4.l = #0xbeef
	memw(r5) = r4
	jump .Lerror

FUNC_END H2K_trap_angel

#endif


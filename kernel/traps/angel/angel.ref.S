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

	// if we're loaded at 0, no need to tmpmap; k0lock instead
	r19 = memw(H2K_GP + #KG_phys_offset)
	p0 = cmp.eq(r19, ##H2K_LINK_ADDR)
	if (p0) jump .Lat0
				
	// save regs clobbered by calls
	r18 = r31
	r17:16 = r1:0
	
	r0 = #0x20
	r1 = #UC_S
	call H2K_tmpmap_add_and_lock
	r6 = r0  // va of mailbox base
	r1:0 = r17:16
	jump .Llocked

.Lat0:
	r6 = ##H2K_LINK_ADDR+0x20
	k0lock

.Llocked:
	r7 = htid
	r8 = #1
	memd(r6+#0) = r1:0
	memw(r6+#8) = r7
	dccleaninva(r6)
	r5:4 = memd(r6+#0)
	p0 = cmp.eq(r5:4,r1:0)
	if (!p0) jump 9f
	memw(r6+#12) = r8
	dccleaninva(r6)
	r8 = memub(r6+#15)
	dccleaninva(r6)
	loop0(1f,#66)
	.falign
1:
	{ nop }:endloop0
	r8 = memw(r6+#12)
	dccleaninva(r6)
	r17:16 = memd(r6+#0)

	p0 = cmp.eq(r19, ##H2K_LINK_ADDR)  // did we tmpmap?
	if (p0) jump .Lno_tmpmap
	call H2K_tmpmap_remove_and_unlock
	r31 = r18  // restore
	jump .Lunlocked

.Lno_tmpmap:
	k0unlock

.Lunlocked:
	r1:0 = r17:16
	jumpr r31
9:
	r5 = #0x1c
	r4.h = #0xdead
	r4.l = #0xbeef
	memw(r5) = r4
	jump 9b

FUNC_END H2K_trap_angel

#endif


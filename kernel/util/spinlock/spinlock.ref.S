/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

FUNC_START H2K_spinlock_lock .text.util.spinlock .p2align 5
	ASM_TICKET_LOCK(r0,r2,r3,p0)
	jumpr r31
FUNC_END H2K_spinlock_lock

FUNC_START H2K_spinlock_trylock .text.util.spinlock .p2align 5
	r1 = memw_locked(r0)
	{
		r2 = lsr(r1,#16)
		r1 = add(r1,##0x10000)
	}
	{
		p0 = cmph.eq(r1,r2)
		if (!p0.new) jump:nt .Lfail
	}
	memw_locked(r0,p0) = r1
	if (!p0) jump H2K_spinlock_trylock
	{
		r0 = #0
		jumpr r31
	}
.Lfail:
	{
		r0 = #1
		jumpr r31
	}
FUNC_END H2K_spinlock_trylock

FUNC_START H2K_spinlock_unlock .text.util.spinlock .p2align 5
	ASM_TICKET_UNLOCK(r0,r2,r3,p0)
	jumpr r31
FUNC_END H2K_spinlock_unlock


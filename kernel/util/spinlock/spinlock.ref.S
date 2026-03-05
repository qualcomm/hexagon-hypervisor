/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>

FUNC_START H2K_spinlock_lock .text.util.spinlock .p2align 5
	r1 = memw_locked(r0)
	if (r1!=#0) jump:nt H2K_spinlock_lock
	memw_locked(r0,p0) = r31
	if (!p0) jump H2K_spinlock_lock
	{
		dccleana(r0)
		r0 = #0
		jumpr r31
	}
FUNC_END H2K_spinlock_lock

FUNC_START H2K_spinlock_trylock .text.util.spinlock .p2align 5
	r1 = memw_locked(r0)
	if (r1!=#0) jump:nt .Lfail
	memw_locked(r0,p0) = r31
	if (!p0) jump H2K_spinlock_trylock
	{
		dccleana(r0)
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
	r1 = #0
	memw(r0) = r1
	{
		dccleana(r0)
		r0 = #0
		jumpr r31
	}
FUNC_END H2K_spinlock_unlock


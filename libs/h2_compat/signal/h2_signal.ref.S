/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

#define OFF_MASK 4
#define OFF_SIGS 0
#define SIGNAL r0
#define MASK r1
#define SIGVAL r2
#define MASKVAL r3
#define MASK_SIGVAL r3:2

	/* 
	 * h2_anysignal_wait(h2_signal_t *signal, unsigned int mask)
	 * Wait for one of the signals specified in mask is set
	 * If already set, return immediately, otherwise block
	 * return value is latest value of signals
	 */
FUNC_START h2_signal_wait_any .text.h2.signal .p2align 5
	MASK_SIGVAL = memd_locked(SIGNAL)
	{
		p0 = bitsclr(SIGVAL,MASK)
		if (!p0.new) MASKVAL = #0
		if (p0.new) MASKVAL = MASK
	}
	memd_locked(SIGNAL,p1) = MASK_SIGVAL
	{
		if (!p1) jump h2_signal_wait_any
	}
	{
		if (!p0) jumpr r31
		if (!p0) r0 = SIGVAL
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r1:0
		r1 = SIGVAL
	}
	trap0(#H2_TRAP_FUTEX_WAIT)
	{
		r1:0 = memd(R29+#0)
		deallocframe
		jump h2_signal_wait_any
	}
FUNC_END h2_signal_wait_any

	/* 
	 * qurt_signal_wait_all(h2_signal_t *signal, unsigned int mask)
	 * Wait for all of the signals specified in mask is set
	 * If already set, return immediately, otherwise block
	 * return value is latest value of signals
	 */
FUNC_START h2_signal_wait_all .text.h2.signal .p2align 4
	MASK_SIGVAL = memd_locked(SIGNAL)
	{
		p0 = bitsset(SIGVAL,MASK)
		MASKVAL = and(MASK,~SIGVAL)
	}
	{
		if (p0) MASKVAL = #0
	}
	memd_locked(SIGNAL,p1) = MASK_SIGVAL
	{
		if (!p1) jump h2_signal_wait_all
	}
	{
		if (p0) jumpr r31
		if (p0) r0 = SIGVAL
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r1:0
		r1 = SIGVAL
	}
	trap0(#H2_TRAP_FUTEX_WAIT)
	{
		r1:0 = memd(R29+#0)
		deallocframe
		jump h2_signal_wait_all
	}
FUNC_END h2_signal_wait_all

FUNC_START h2_signal_set .text.h2.signal .p2align 5
	MASK_SIGVAL = memd_locked(SIGNAL)
	{
		SIGVAL = or(SIGVAL,MASK)
		p0 = bitsclr(MASKVAL,MASK)
	}
	memd_locked(SIGNAL,p1) = MASK_SIGVAL
	{
		if (!p1) jump h2_signal_set
	}
	/* If we didn't contribute anything that was being waited on, return early */
	{
		if (p0) jumpr r31
		if (p0) r0 = SIGVAL
		r1 = #99
	}
	/* Wake up some threads waiting on this signal */
	{
		allocframe(#8)
		memd(r29+#-16) = r17:16
		r17:16 = combine(r1,r0)
	}
.Ldo_trap:
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		p0 = cmp.gt(r0,#-1)
		if (!p0.new) r1:0 = r17:16
		if (!p0.new) r3:2 = memd(r16)
		if (!p0.new) jump:nt .Ldo_trap
	}
	{
		r17:16 = memd(r29)
		r0 = SIGVAL 
		dealloc_return
	}
FUNC_END h2_signal_set


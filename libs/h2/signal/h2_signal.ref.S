/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

#define OFF_MASK 4
#define OFF_SIGS 0
#define SIGNAL r0
#define MASK r1
#define SIGVAL r2
#define MASKVAL r3
#define MASK_SIGVAL r3:2

	.text

	.global h2_signal_wait_any
	.p2align 5
	/* 
	 * h2_anysignal_wait(h2_signal_t *signal, unsigned int mask)
	 * Wait for one of the signals specified in mask is set
	 * If already set, return immediately, otherwise block
	 * return value is latest value of signals
	 */
h2_signal_wait_any:
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
	.size h2_signal_wait_any,.-h2_signal_wait_any

	.global h2_signal_wait_all
	.p2align 5
	/* 
	 * qurt_signal_wait_all(h2_signal_t *signal, unsigned int mask)
	 * Wait for all of the signals specified in mask is set
	 * If already set, return immediately, otherwise block
	 * return value is latest value of signals
	 */
h2_signal_wait_all:
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
	.size h2_signal_wait_all,.-h2_signal_wait_all

	.global h2_signal_set
h2_signal_set:
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
		if (p0) r0 = r2
		r1 = #99
	}
	/* Wake up some threads waiting on this signal */
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		r0 = r2
		jumpr r31
	}
	.size h2_signal_set,.-h2_signal_set


/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

#define OFF_MASK 4
#define OFF_SIGS 0
#define SIGNAL r0
#define MASK r1
#define TMP r2
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
	TMP = memw(SIGNAL+#OFF_SIGS)
	{
		p0 = bitsclr(TMP,MASK)
		if (!p0.new) r0 = TMP
		if (!p0.new) jumpr:nt r31
	}
	{
		allocframe(#8)
		memw(SIGNAL+#OFF_MASK) = r1
	}
	{
		memd(r29+#0) = r1:0
		r1 = TMP
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
	TMP = memw(SIGNAL+#OFF_SIGS)
	{
		p0 = bitsset(TMP,MASK)
		if (p0.new) r0 = TMP
		if (p0.new) jumpr:nt r31
	}
	{
		allocframe(#8)
		memw(SIGNAL+#OFF_MASK) = r1
	}
	{
		memd(r29+#0) = r1:0
		r1 = TMP
	}
	trap0(#H2_TRAP_FUTEX_WAIT)
	{
		r1:0 = memd(R29+#0)
		deallocframe
		jump h2_signal_wait_any
	}
	.size h2_signal_wait_all,.-h2_signal_wait_all

	.global h2_signal_set
h2_signal_set:
	r3:2 = memd_locked(SIGNAL)
	{
		r2 = or(r2,MASK)
		p0 = bitsclr(r3,MASK)
	}
	memd_locked(SIGNAL,p1) = r3:2
	{
		if (!p1) jump h2_signal_set
	}
	/* If we didn't contribute anything that was being waited on, return early */
	{
		if (p0) jumpr r31
		if (p0) r0 = r2
		r1 = #1
	}
	/* Wake up a thread waiting on this signal */
	trap0(#H2_TRAP_FUTEX_WAKE)
	{
		r0 = r2
		jumpr r31
	}
	.size h2_signal_set,.-h2_signal_set


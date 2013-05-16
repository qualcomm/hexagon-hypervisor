/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

	.text
	.p2align 12
TH_vecs:
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_nonint_vec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec
	jump TH_intvec

TH_nonint_vec:
	nop
	rte

TH_intvec:
	allocframe(#0)
	r2 = sgp0
	r1.h = #hi(a)
	r1.l = #lo(a)
	p0 = cmp.eq(r2,#1)
	p1 = cmp.eq(r0,r1)
	p0 = and(p0,p1)
	r0 = mux(p0,#1,#0)
	sgp0 = r0
	call TH_interrupt
	deallocframe
	rte

	.global TH_prepare
TH_prepare:
	/* Prepare SSR: EX+IE */
	r0 = ssr
	r1 = #6
	r0 = insert(r1,#3,#16)
	ssr = r0

	r1.h = #hi(TH_vecs)
	r1.l = #lo(TH_vecs)
	evb = r1

	r1 = #-1
	iel = r1
	iahl = r1
	cswi(r1)
	ciad(r1)

	jumpr r31

	.global TH_imask
TH_imask:
	imask = r0
	jumpr r31

	.global TH_pend_int
TH_pend_int:
	r1 = #1
#if ARCHV <= 3
	r1 = brev(r1)
#endif
	swi(r1)
	jumpr r31

	.global TH_call_preempt
TH_call_preempt:
	allocframe(#8)
	memd(r29) = r17:16
	r17 = sgp0
	sgp0 = r0
	call H2K_preempt
	sgp0 = r17
	r17:16 = memd(r29)
	deallocframe
	jumpr r31


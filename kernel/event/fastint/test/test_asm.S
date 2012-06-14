/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

	.text
	.p2align 12
myvecs:
	jump TH_bad_interrupt
	jump TH_bad_interrupt //event_handle_error
	jump TH_goto_event_handle_error //event_handle_error
	jump TH_bad_interrupt
	jump event_handle_tlbmissx
	jump TH_bad_interrupt
	jump event_handle_tlbmissrw
	jump TH_bad_interrupt
	jump TH_goto_event_handle_trap0 
	jump TH_bad_interrupt
	jump TH_bad_interrupt
	jump TH_bad_interrupt
	jump TH_bad_interrupt
	jump TH_bad_interrupt
	jump TH_bad_interrupt
	jump TH_bad_interrupt
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int
	jump TH_call_saw_secondary_int

TH_goto_event_handle_trap0:
	r15 = memw(#oldvecs)
	r15 = add(r15,#4*8)
	jumpr r15

TH_goto_event_handle_error:
	r15 = memw(#oldvecs)
	r15 = add(r15,#4*2)
	jumpr r15

TH_call_saw_secondary_int:
	r0 = ssr
	r0 = and(r0,#0x1f)
	r1 = #0
	r1 = setbit(r1,r0)
#if __QDSP6_ARCH__ <= 3
	r1 = brev(r1)
#endif
	ciad(r1)
	r29 = memw(#stacksave)
	r29 = add(r29,#-32)
	r28 = memw(#ssrsave)
	ssr = r28
	jump TH_saw_secondary_int

// void TH_call_fastint_intpending(u32_t intno, H2K_thread_context *me, u32_t int2);
	.global TH_call_fastint_intpending
TH_call_fastint_intpending:
	allocframe(#8)
	memw(#stacksave) = r29
	r28 = ssr
	memw(#ssrsave) = r28
	r5 = memw(#oldvecs)
	p0 = cmp.eq(r5,#0)
	if (!p0) jump 1f
	r5 = evb
	memw(#oldvecs) = r5
1:
	r5.h = #hi(myvecs)
	r5.l = #lo(myvecs)
	evb = r5
	r4 = #0
	r4 = setbit(r4,r2)
#if __QDSP6_ARCH__ <= 3
	r4 = brev(r4)
#endif
	swi(r4)
// sibcall h2K_fastint(intno,me,hthread)
	r2 = #0
	call H2K_fastint
	deallocframe
	jumpr r31
	

	.global TH_fastint_call
TH_fastint_call:
	allocframe(#8)
	r4 = sgp
	memd(r29+#0) = r5:4
	sgp = r1
	memw(#stacksave) = r29
	call H2K_fastint
	r29 = memw(#stacksave)
	r5:4 = memd(r29+#0)
	sgp = r4
	deallocframe
	jumpr r31

	.section .sdata,"aw"
	.p2align 2
oldvecs:	.word 0
stacksave:	.word 0
stacksave2:	.word 0
ssrsave:	.word 0
	.p2align 3
lrfpsave:	.word 0,0


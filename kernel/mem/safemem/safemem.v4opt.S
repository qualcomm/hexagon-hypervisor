/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

FUNC_START H2K_safemem_check_and_lock  .text.core.futex  .p2align  5
/* u32_t 
 * H2K_safemem_check_and_lock(void *user_va, u32_t perms, pa_t *pa_ret, H2K_thread_context *me)
 */
#define VA r0
#define PERMS_IN r1
#define PA_RET r2
#define ME r3
#define SSRVAL r4
#define IDX R5
#define TMPPAIR_HI r7
#define TMPPAIR_LO r6
#define TMPPAIR r7:6
#define PERMS r8
#define TLBPERMS r9
#define SIZE r10
#define WIDTH r11
#define WIDTHSIZE r11:10
#define JUNK_PPN r13:12
#define PPN r12
#define JUNK r13
#define DSTPAIR r15:14
#define DSTPAIR_LO r14
#define DSTPAIR_HI r15
	{
		p0 = bitsclr(VA,#3)
		if (!p0.new) jump:nt .Lbad
		SSRVAL = memw(ME+#CONTEXT_ssr)
		PERMS_IN = add(PERMS_IN,PERMS_IN)	// shift left by 1
	}
	tlblock
	{
		TMPPAIR_HI = extractu(SSRVAL,#8,#8)	// get asid
		PERMS = extractu(SSRVAL,#1,#19)		// get Guest Mode
		TMPPAIR_LO = VA
		PERMS_IN = add(PERMS_IN,#1)
	}
	{
		TMPPAIR = lsr(TMPPAIR,#12)		// get ASID:VPN
		PERMS = sub(PERMS_IN,PERMS)		// LSB User Mode
		DSTPAIR = combine(#0,VA)
	}
	{
		IDX = TLBP(TMPPAIR_LO)
	}
	{
		TMPPAIR = TLBR(IDX)			// may read junk
	}
	{
		IDX = IDX				// get R.new value
		if (!cmp.gt(IDX.new,#-1)) jump:nt .Lbad_locked
		TLBPERMS = extractu(TMPPAIR_LO,#4,#28)
		SIZE = ct0(TMPPAIR_LO)
	}
#undef IDX
#define PA35 r5
	{
		TLBPERMS = and(TLBPERMS,PERMS)
		if (!cmp.eq(TLBPERMS.new,PERMS)) jump:nt .Lbad_locked
		SIZE = add(#1,asl(SIZE,#1))		// 2*SIZE+1
		PA35 = extractu(TMPPAIR_HI,#1,#29)
	}
	{
		DSTPAIR_HI = insert(PA35,#1,#35-32)	// insert PA35
		PPN = lsr(TMPPAIR_LO,SIZE)
		SIZE = add(SIZE,#(12-1))		// 2*size+12
		WIDTH = sub(#24,SIZE)			// 23-2*size
	}
	{
		DSTPAIR = insert(JUNK_PPN,WIDTHSIZE)
	}
	{
		memd(PA_RET) = DSTPAIR
		r0 = #1
		jumpr r31
	}
.Lbad_locked:
	tlbunlock
.Lbad:
	{
		r0 = #0
		jumpr r31
	}
FUNC_END H2K_safemem_check_and_lock


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
#define PPN r5
	{
		TLBPERMS = and(TLBPERMS,PERMS)
		if (!cmp.eq(TLBPERMS.new,PERMS)) jump:nt .Lbad_locked
		SIZE = add(#1,asl(SIZE,#1))		// 2*SIZE+1
	}
	{
		PPN = lsr(TMPPAIR_LO,SIZE)
		SIZE = add(SIZE,#(12-1))		// 2*size+12
		WIDTH = sub(#21,SIZE)			// 20-2*size
		TMPPAIR_HI = VA
	}
	{
		TMPPAIR_HI = insert(PPN,WIDTHSIZE)
		memw(PA_RET) = TMPPAIR_HI.NEW
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


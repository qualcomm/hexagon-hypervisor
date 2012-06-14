/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

/*
H2K_check_sanity, v4 optimized

R0 - Do not touch.
R1 - RESCHED_INT_INTMASK
R2 - priomask
R3 - waitmask
R4 - ready_valids / runlist_prios
R5 - ready_valids / runlist_prios
R6 - runlist_worst_prio()
R7 - hthread
R8 - ready_valids / ready_best_prio()
R9 - ready_valids
R10 - ready_valids
R11 - ready_valids
R12 - ready_valids
R13 - ready_valids
R14 - temp
R15 - temp
R28 - temp

p0 - wait_mask == 0? / skip sched
p1 - !H2K_ready_any_valid()
p2 - do notify?
p3 - thread for setimask

Best case:  8 cycles
Worst case:  13 cycles + 1 cycle mispredict penalty
*/

#define INPUT r1:0
#define READY_VALIDS_0	r15:14
#define READY_VALIDS_1	r13:12
#define TZ_0		r11
#define TZ_1		r10
#define TZ_2		r9
#define TZ_3		r8
#define RUNLIST_PRIOS_0 r7:6
#define MAXRESULTS r5:4
#define MAXRESULTS_HI r5
#define MAXRESULTS_LO r4
#define TMP0 r2
#define TMP1 r3

FUNC_START H2K_check_sanity_unlock .text.core.sanity .p2align 5
	{
		READY_VALIDS_0 = memd(H2K_GP + #KG_ready_valids)				//  H2K_gp->ready_valids[0]
		READY_VALIDS_1 = memd(H2K_GP + #KG_ready_valids + 8)				//  H2K_gp->ready_valids[1]
		MAXRESULTS = combine(#0,#-1)							//  initial hthread : MAX(runlist_prio)
		TMP0 = #4									//  initial hthread if >= 4
	}
	{
		TZ_0 = ct0(READY_VALIDS_0)
		TZ_1 = ct0(READY_VALIDS_1)
		READY_VALIDS_0 = memd(H2K_GP + #KG_ready_valids + 16)				//  H2K_gp->ready_valids[2]
		READY_VALIDS_1 = memd(H2K_GP + #KG_ready_valids + 24)				//  H2K_gp->ready_valids[3]
	}
	{
		p0 = cmp.eq(TZ_0,#64)
		TZ_2 = ct0(READY_VALIDS_0)
		TZ_3 = ct0(READY_VALIDS_1)
		RUNLIST_PRIOS_0 = memd(H2K_GP + #KG_runlist_prios)				//  H2K_gp->runlist_prios[0-3]
	}
#undef READY_VALIDS_0
#undef READY_VALIDS_1
#define RUNLIST_PRIOS_1 r15:14
#define MASKPAIR r13:12
#define WAITMASK r13
#define PRIOMASK r12
	{
		if (p0) TZ_0 = add(TZ_0,TZ_1)
		MAXRESULTS = vrmaxh(RUNLIST_PRIOS_0,MAXRESULTS_HI)				//  hthread << 1
		RUNLIST_PRIOS_1 = memd(H2K_GP + #KG_runlist_prios + 8)				//  H2K_gp->runlist_prios[4-7]
		MASKPAIR = memd(H2K_GP + #KG_lowprio_masks)					//  wait_mask : priomask
	}
#undef TZ_1
#define HTHREAD r10
	{
		p0 = cmp.eq(TZ_0,#128)
		if (p0.new) TZ_0 = add(TZ_0,TZ_2)
		MAXRESULTS = vrmaxh(RUNLIST_PRIOS_1,TMP0)					//  hthread << 1
		p2 = cmp.eq(PRIOMASK,#0)							//  priomask == 0?
	}
#undef TZ_2
	{
		p0 = cmp.eq(TZ_0,#192)
		if (p0.new) TZ_0 = add(TZ_0,TZ_3)						//  ready_best_prio()
		HTHREAD = lsr(MAXRESULTS_HI,#1)							//  hthread
	}
#undef TZ_3
	{
		p1 = cmp.eq(TZ_0,#256)								//  !H2K_ready_any_valid()
		p0 = cmp.eq(WAITMASK,#0)							//  wait_mask
		if (!p2) jump 1f
	}
	//  do lowprio_notify()
	{
		PRIOMASK = setbit(PRIOMASK,HTHREAD)						//  set new priomask
		memw(H2K_GP+#KG_priomask) = PRIOMASK.new					//  set new priomask
		p3 = HTHREAD									//  prepare setimask
		TMP0 = #0									//  0 prepare setimask
	}
	{
		setimask(p3,TMP0)								//  set register somewhere
	}
	.falign
1:
	{
		p0 = or(p0,p1)									//  building a reason to bail out
		p0 = !cmp.gt(MAXRESULTS_LO,TZ_0)						//  last reason to bail out
		if (p0.new) jump:t 2f
		R1 = #RESCHED_INT_INTMASK							//  prepare resched_int
	}
	{
		swi(R1)		//  resched_int()
	}
	.falign
2:
	{
		k0unlock
	}
	{
		jumpr R31
	}
FUNC_END H2K_check_sanity_unlock

/*
H2K_check_sanity, v4 optimized

This is rarely called, so just call check_sanity_unlock and re-lock.

*/

FUNC_START H2K_check_sanity .text.core.sanity .p2align 3
	{
		allocframe(#0)
		call H2K_check_sanity_unlock
	}
	k0lock
	dealloc_return
FUNC_END H2K_check_sanity

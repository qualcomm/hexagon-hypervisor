/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

/*
H2K_check_sanity, v4 optimized

R0 - do not touch.
R1 - temp
R2 - priomask
R3 - waitmask
R4 - hthread
R6 - runlist_worst_prio()
R8 - ready_best_prio()
R9 - ready_valids
R10 - RESCHED_INT_INTMASK

p0 - DO_SCHED?
p1 - DO_NOTIFY?
p3 - thread for setimask

Best case:  7 cycles
Worst case:  10 cycles + 2 cycle mispredict penalty
*/

FUNC_START H2K_check_sanity_unlock .text.core.sanity .p2align 5
	{
		R5:4 = memd(H2K_GP + #KG_runlist_prios)
		R7:6 = combine(#0,#-1)
		R11 = #4
		R9 = memw(H2K_GP + #KG_ready_valids);	//  ready_valids
	}
	{
		R7:6 = vrmaxh(R5:4,R7)
		R5:4 = memd(H2K_GP + #KG_runlist_prios + 8)
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);
		R10 = #RESCHED_INT_INTMASK;		//  prepare resched_int
	}
	{
		R7:6 = vrmaxh(R5:4,R11)
		p0 = !cmp.eq(R2,#0);			//  priomask == 0?
		if (p0.new) jump:t 3f;
	}
	//  do lowprio_notify
	{
		R4 = lsr(R7,#1)
		R8 = ct0(R9);			//  ready_best_prio()
		p0 = cmp.eq(R3,#0);		//  D - wait_mask
		p1 = cmp.eq(R9,#0);		//  ready_valids
	}
	{
		R2 = setbit(R2,R4);			//  set new priomask
		memw(H2K_GP+#KG_priomask) = R2.new;	//  set new priomask
		p3 = R4;				//  prepare setimask
		R5 = #0;			//  D - 0 prepare setimask
	}
	{
		setimask(p3,R5);  //  set register somewhere
	}
	{
		p0 = or(p0,p1);		//  building a reason to bail out
		p0 = !cmp.gt(R6,R8);	//  last reason to bail out
		if (p0.new) jump:t 2f;
	}  //  just did like 80 bajillion comparisons in one packet
	.falign
1:
	{
		swi(R10);
	}
	.falign
2:
	{
		k0unlock
	}
	{
		jumpr r31;
	}
	.falign
3:
	{
		R8 = ct0(R9);		//  ready_best_prio()
		p0 = cmp.eq(R3,#0);	//  wait_mask
		p1 = cmp.eq(R9,#0);	//  ready_valids
	}  //  only need to check the two 'if' clauses now
	{
		p0 = !cmp.gt(R6,R8);	//  is_worse_than?
		p0 = or(p0,p1);
		if (!p0.new) jump:t 1b;  //  this should be a single compare-jump instruction, not that it matters.
	}
	{
		k0unlock
	}
	{
		jumpr r31;
	}
FUNC_END H2K_check_sanity_unlock

//  crappy cut and paste until we figure out what to do exactly.

/*
H2K_check_sanity, v4 optimized

R0 - do not touch.
R1 - temp
R2 - priomask
R3 - waitmask
R4 - hthread
R6 - runlist_worst_prio()
R8 - ready_best_prio()
R9 - ready_valids
R10 - RESCHED_INT_INTMASK

p0 - DO_SCHED?
p1 - DO_NOTIFY?
p3 - thread for setimask
*/

FUNC_START H2K_check_sanity .text.core.sanity .p2align 5
	{
		R5:4 = memd(H2K_GP + #KG_runlist_prios)
		R7:6 = combine(#0,#-1)
		R11 = #4
		R9 = memw(H2K_GP + #KG_ready_valids);	//  ready_valids
	}
	{
		R7:6 = vrmaxh(R5:4,R7)
		R5:4 = memd(H2K_GP + #KG_runlist_prios + 8)
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);
		R10 = #RESCHED_INT_INTMASK;		//  prepare resched_int
	}
	{
		R7:6 = vrmaxh(R5:4,R11)
		p0 = !cmp.eq(R2,#0);			//  priomask == 0?
		if (p0.new) jump:t 3f;
	}
	//  do lowprio_notify
	{
		R4 = lsr(R7,#1)
		R8 = ct0(R9);			//  ready_best_prio()
		p0 = cmp.eq(R3,#0);		//  D - wait_mask
		p1 = cmp.eq(R9,#0);		//  ready_valids
	}
	{
		R2 = setbit(R2,R4);			//  set new priomask
		memw(H2K_GP+#KG_priomask) = R2.new;	//  set new priomask
		p3 = R4;				//  prepare setimask
		R5 = #0;			//  D - 0 prepare setimask
	}
	{
		setimask(p3,R5);  //  set register somewhere
	}
	{
		p0 = or(p0,p1);		//  building a reason to bail out
		p0 = !cmp.gt(R6,R8);	//  last reason to bail out
		if (p0.new) jump:t 2f;
	}  //  just did like 80 bajillion comparisons in one packet
	.falign
1:
	{
		swi(R10);
	}
	.falign
2:
	{
		jumpr r31;
	}
	.falign
3:
	{
		R8 = ct0(R9);		//  ready_best_prio()
		p0 = cmp.eq(R3,#0);	//  wait_mask
		p1 = cmp.eq(R9,#0);	//  ready_valids
	}  //  only need to check the two 'if' clauses now
	{
		p0 = !cmp.gt(R6,R8);	//  is_worse_than?
		p0 = or(p0,p1);
		if (!p0.new) jump:t 1b;  //  this should be a single compare-jump instruction, not that it matters.
	}
	{
		jumpr r31;
	}
FUNC_END H2K_check_sanity

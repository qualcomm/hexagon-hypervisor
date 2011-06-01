/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

/*
H2K_check_sanity, v3 optimized

Registers clobbered:

R1 - temp
R2 - priomask
R3 - waitmask
R4 - runlist_worst_prio_hthread()
R5 - hthread, imask param
R6 - H2K_GP->runlist, hthread
R7 - runlist_worst_prio()
R8 - ready_best_prio()
R9 - ready_valids
R10 - #RESCHED_INT_INTMASK
R11 - ready_best_prio()

P0 - DO_NOTIFY?, DO_SCHED?
P1 - DO_SCHED?
P2 - DO_BOTH?
P3 - thread for setimask

Best case:  13 cycles
Worst case:  17 cycles + 1 cycle mispredict penalty
*/

FUNC_START H2K_check_sanity_unlock .text.core.sanity .p2align 5
	{
		R9:8 = memd(H2K_GP+#KG_ready_valids)       // H2K_gp->ready_valids[0]
		R3:2 = memd(H2K_GP+#KG_ready_valids + 8)   // H2K_gp->ready_valids[1]
	}
	{
		R5:4 = memd(H2K_GP+#KG_ready_valids + 16)  // H2K_gp->ready_valids[2]
		R7:6 = memd(H2K_GP+#KG_ready_valids + 24)  // H2K_gp->ready_valids[3]
		R8 = ct0(R8)
	}
	{
		p0 = cmp.gt(R8,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R9)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R2)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R3)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R4)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R5)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R6)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R7)
	}
	{
		R8 = add(R8,R1)  // ready_best_prio()
	}
	.falign
1:
	{
		R7 = #-1
		R4 = #-1
		R1 = memh(H2K_GP + #KG_runlist_prios)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #0
		R1 = memh(H2K_GP + #KG_runlist_prios + 2)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #1
		R1 = memh(H2K_GP + #KG_runlist_prios + 4)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #2
		R1 = memh(H2K_GP + #KG_runlist_prios + 6)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #3
		R1 = memh(H2K_GP + #KG_runlist_prios + 8)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #4
		R1 = memh(H2K_GP + #KG_runlist_prios + 10)
	}
	{
		p2 = cmp.gt(R1,R7)
		R9 = memw(H2K_GP + #KG_ready_valids);	//  ready_valids
		R10.H = #HI(RESCHED_INT_INTMASK);	//  prepare resched_int
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);
	}
	{
		if (p2) R7 = R1		//  runlist_worst_prio()
		R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int
		p0 = cmp.eq(R2,#0);			//  Duplex - priomask == 0?
		if (!p0.new) jump:t 3f;
	}
	//  do lowprio_notify
	{
		if (p2) R4 = #5		//  runlist_worst_prio_hthread()
		p0 = cmp.eq(R3,#0);		//  D - wait_mask
		p1 = cmp.eq(R8,#256);		//  ready_valids
	}
	{
		R2 = setbit(R2,R4);			//  set new priomask
		p3 = R4;				//  prepare setimask
		R5 = #0;			//  D - 0 prepare setimask
	}
	{
		memw(H2K_GP+#KG_priomask) = R2;	//  set new priomask
	}
	{
		setimask(p3,R5);  //  set register somewhere
	}
	{
		p0 = or(p0,p1);		//  building a reason to bail out
		p1 = cmp.gt(R7,R8);	//  last reason to bail out
	}
	{
		p0 = and(p0,!p1)
		if (p0.new) jump:t 2f;
	}
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
		jumpr R31;
	}
	.falign
3:
	{
		p0 = cmp.eq(R3,#0);	//  wait_mask
		p1 = cmp.eq(R8,#256);	//  ready_valids
	}
	//  only need to check the two 'if' clauses now
	{
		p1 = cmp.gt(R7,R8);	//  is_worse_than?
		p0 = or(p0,p1);
	}
	{
		p0 = and(p0,!p1)
		if (!p0.new) jump:t 1b;
	}
	{
		k0unlock
	}
	{
		jumpr R31;
	}
FUNC_END H2K_check_sanity_unlock

//  crappy cut and paste until we figure out what to do exactly.
FUNC_START H2K_check_sanity .text.core.sanity .p2align 5
	{
		R9:8 = memd(H2K_GP+#KG_ready_valids)       // H2K_gp->ready_valids[0]
		R3:2 = memd(H2K_GP+#KG_ready_valids + 8)   // H2K_gp->ready_valids[1]
	}
	{
		R5:4 = memd(H2K_GP+#KG_ready_valids + 16)  // H2K_gp->ready_valids[2]
		R7:6 = memd(H2K_GP+#KG_ready_valids + 24)  // H2K_gp->ready_valids[3]
		R8 = ct0(R8)
	}
	{
		p0 = cmp.gt(R8,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R9)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R2)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R3)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R4)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R5)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R6)
	}
	{
		R8 = add(R8,R1)
		p0 = cmp.gt(R1,#31)
		if (!p0.new) jump:t 1f
		R1 = ct0(R7)
	}
	{
		R8 = add(R8,R1)  // ready_best_prio()
	}
	.falign
1:
	{
		R7 = #-1
		R4 = #-1
		R1 = memh(H2K_GP + #KG_runlist_prios)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #0
		R1 = memh(H2K_GP + #KG_runlist_prios + 2)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #1
		R1 = memh(H2K_GP + #KG_runlist_prios + 4)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #2
		R1 = memh(H2K_GP + #KG_runlist_prios + 6)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #3
		R1 = memh(H2K_GP + #KG_runlist_prios + 8)
	}
	{
		p0 = cmp.gt(R1,R7)
		if (p0.new) R7 = R1
		if (p0.new) R4 = #4
		R1 = memh(H2K_GP + #KG_runlist_prios + 10)
	}
	{
		p2 = cmp.gt(R1,R7)
		R9 = memw(H2K_GP + #KG_ready_valids);	//  ready_valids
		R10.H = #HI(RESCHED_INT_INTMASK);	//  prepare resched_int
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);
	}
	{
		if (p2) R7 = R1		//  runlist_worst_prio()
		R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int
		p0 = cmp.eq(R2,#0);			//  Duplex - priomask == 0?
		if (!p0.new) jump:t 3f;
	}
	//  do lowprio_notify
	{
		if (p2) R4 = #5		//  runlist_worst_prio_hthread()
		p0 = cmp.eq(R3,#0);		//  D - wait_mask
		p1 = cmp.eq(R8,#256);		//  ready_valids
	}
	{
		R2 = setbit(R2,R4);			//  set new priomask
		p3 = R4;				//  prepare setimask
		R5 = #0;			//  D - 0 prepare setimask
	}
	{
		memw(H2K_GP+#KG_priomask) = R2;	//  set new priomask
	}
	{
		setimask(p3,R5);  //  set register somewhere
	}
	{
		p0 = or(p0,p1);		//  building a reason to bail out
		p1 = cmp.gt(R7,R8);	//  last reason to bail out
	}
	{
		p0 = and(p0,!p1)
		if (p0.new) jump:t 2f;
	}
	.falign
1:
	{
		swi(R10);
	}
	.falign
2:
	{
		jumpr R31;
	}
	.falign
3:
	{
		p0 = cmp.eq(R3,#0);	//  wait_mask
		p1 = cmp.eq(R8,#256);	//  ready_valids
	}
	//  only need to check the two 'if' clauses now
	{
		p1 = cmp.gt(R7,R8);	//  is_worse_than?
		p0 = or(p0,p1);
	}
	{
		p0 = and(p0,!p1)
		if (!p0.new) jump:t 1b;
	}
	{
		jumpr R31;
	}
FUNC_END H2K_check_sanity

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

FUNC_START H2K_check_sanity_unlock .text.core.sanity .p2align 5
	{
		R9:8 = memd(H2K_GP + #KG_ready_valids)         //  H2K_gp->ready_valids[0]
		R11:10 = memd(H2K_GP + #KG_ready_valids + 8)   //  H2K_gp->ready_valids[1]
		R7:6 = combine(#0,#-1)                         //  initial hthread : MAX(runlist_prio)
		R28 = #4                                       //  initial hthread if >= 4
	}
	{
		R8 = ct0(R9:8)
		R1 = ct0(R11:10)
		R11:10 = memd(H2K_GP + #KG_ready_valids + 16)  //  H2K_gp->ready_valids[2]
		R13:12 = memd(H2K_GP + #KG_ready_valids + 24)  //  H2K_gp->ready_valids[3]
	}
	{
		p0 = cmp.eq(R8,#64)
		R14 = ct0(R11:10)
		R15 = ct0(R13:12)
		R5:4 = memd(H2K_GP + #KG_runlist_prios)        //  H2K_gp->runlist_prios[0-3]
	}
	{
		if (p0) R8 = add(R8,R1)
		R7:6 = vrmaxh(R5:4,R7)                         //  hthread << 1
		R5:4 = memd(H2K_GP + #KG_runlist_prios + 8)    //  H2K_gp->runlist_prios[4-7]
		R3:2 = memd(H2K_GP + #KG_lowprio_masks)        //  wait_mask : priomask
	}
	{
		p0 = cmp.eq(R8,#128)
		if (p0.new) R8 = add(R8,R14)
		R7:6 = vrmaxh(R5:4,R28)                        //  hthread << 1
		p2 = cmp.eq(R2,#0)                             //  priomask == 0?
	}
	{
		p0 = cmp.eq(R8,#192)
		if (p0.new) R8 = add(R8,R15)                   //  ready_best_prio()
		R7 = lsr(R7,#1)                                //  hthread
	}
	{
		p1 = cmp.eq(R8,#256)                           //  !H2K_ready_any_valid()
		p0 = cmp.eq(R3,#0)                             //  wait_mask
		if (!p2) jump 1f
	}
	//  do lowprio_notify()
	{
		R2 = setbit(R2,R7)                             //  set new priomask
		memw(H2K_GP+#KG_priomask) = R2.new             //  set new priomask
		p3 = R7                                        //  prepare setimask
		R5 = #0                                        //  0 prepare setimask
	}
	{
		setimask(p3,R5)                                //  set register somewhere
	}
	.falign
1:
	{
		p0 = or(p0,p1)                                 //  building a reason to bail out
		p0 = !cmp.gt(R6,R8)                            //  last reason to bail out
		if (p0.new) jump:t 2f
		R1 = #RESCHED_INT_INTMASK                      //  prepare resched_int
	}
	{
		swi(R1)                                        //  resched_int()
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
*/

FUNC_START H2K_check_sanity .text.core.sanity .p2align 5
	{
		R9:8 = memd(H2K_GP + #KG_ready_valids)         //  H2K_gp->ready_valids[0]
		R11:10 = memd(H2K_GP + #KG_ready_valids + 8)   //  H2K_gp->ready_valids[1]
		R7:6 = combine(#0,#-1)                         //  initial hthread : MAX(runlist_prio)
		R28 = #4                                       //  initial hthread if >= 4
	}
	{
		R8 = ct0(R9:8)
		R1 = ct0(R11:10)
		R11:10 = memd(H2K_GP + #KG_ready_valids + 16)  //  H2K_gp->ready_valids[2]
		R13:12 = memd(H2K_GP + #KG_ready_valids + 24)  //  H2K_gp->ready_valids[3]
	}
	{
		p0 = cmp.eq(R8,#64)
		R14 = ct0(R11:10)
		R15 = ct0(R13:12)
		R5:4 = memd(H2K_GP + #KG_runlist_prios)        //  H2K_gp->runlist_prios[0-3]
	}
	{
		if (p0) R8 = add(R8,R1)
		R7:6 = vrmaxh(R5:4,R7)                         //  hthread << 1
		R5:4 = memd(H2K_GP + #KG_runlist_prios + 8)    //  H2K_gp->runlist_prios[4-7]
		R3:2 = memd(H2K_GP + #KG_lowprio_masks)        //  wait_mask : priomask
	}
	{
		p0 = cmp.eq(R8,#128)
		if (p0.new) R8 = add(R8,R14)
		R7:6 = vrmaxh(R5:4,R28)                        //  hthread << 1
		p2 = cmp.eq(R2,#0)                             //  priomask == 0?
	}
	{
		p0 = cmp.eq(R8,#192)
		if (p0.new) R8 = add(R8,R15)                   //  ready_best_prio()
		R7 = lsr(R7,#1)                                //  hthread
	}
	{
		p1 = cmp.eq(R8,#256)                           //  !H2K_ready_any_valid()
		p0 = cmp.eq(R3,#0)                             //  wait_mask
		if (!p2) jump 1f
	}
	//  do lowprio_notify()
	{
		R2 = setbit(R2,R7)                             //  set new priomask
		memw(H2K_GP+#KG_priomask) = R2.new             //  set new priomask
		p3 = R7                                        //  prepare setimask
		R5 = #0                                        //  0 prepare setimask
	}
	{
		setimask(p3,R5)                                //  set register somewhere
	}
	.falign
1:
	{
		p0 = or(p0,p1)                                 //  building a reason to bail out
		p0 = !cmp.gt(R6,R8)                            //  last reason to bail out
		if (p0.new) jumpr:t R31
		R1 = #RESCHED_INT_INTMASK                      //  prepare resched_int
	}
	{
		swi(R1)                                        //  resched_int()
	}
	{
		jumpr R31
	}
FUNC_END H2K_check_sanity

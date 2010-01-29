/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

	.text
	.p2align 2
	.globl H2K_check_sched_mask
	.type	H2K_check_sched_mask, @function

//  10 cycles worst case + 1 mispredict penalty, 4 cycles best, 1 extra reg (R4) Clobberellad
//  vs 14 worst, 5 best
H2K_check_sched_mask:
	{
		R2 = memw(H2K_GP+#KG_runlist_valids);
		R1 = memw(H2K_GP+#KG_ready_validmask);
	}
	{
		R1 = not(R1);   //  ~ready_validmask
		R0 = cl0(R2);   //  runlist_worst_prio()
	}
	{
		R0 = sub(#MAX_PRIO,R0);	//  runlist_worst_prio()
		P0 = bitsclr(R1,R2);
		if (P0.new) jump:t 1f;	//  nothing to do 
	}
	{
		R0 = addasl(H2K_GP,R0,#2);	//  assume 32bit ptr; start hthread
		R4 = memw(H2K_GP+#KG_priomask);	//  load priomask
		R2 = #0;			//  set imask param
	}
	{
		R1 = memw(R0+#KG_runlist);	//  thread_context ptr
	}
	{
		R1 = memub(R1+#CONTEXT_hthread);//  load hthread
	}
	{	
		R4 = setbit(R4,R1);		//  set priomask
		P0 = R1;			//  set imask param
		R3.H = #HI(RESCHED_INT_MASK);
	}
	{
		memw(H2K_GP+#KG_priomask) = R4; //  save priomask
		R3.L = #LO(RESCHED_INT_MASK);
	}
	{
		setimask(P0,R2);
	}
	{
		swi(R3);
	}
1:
	jumpr R31;
	.size	H2K_check_sched_mask, .-H2K_check_sched_mask

/*
H2K_check_sanity, v3 optimized

Registers clobberifimicated irregardlessly:

R1 - runlist_valids, thread_context pointer
R2 - waitmask
R3 - priomask
R4 - ready_validmask, imask param
R5 - ready_valids
R6 - H2K_gp->runlist, hthread
R7 - #RESCHED_INT_MASK
R8 - ~ready_validmask
R9 - runlist_worst_prio()
R10 - ready_valids & ready_validmask
R11 - ready_best_prio()

P0 - DO_NOTIFY?, DO_SCHED?
P1 - DO_SCHED?
P2 - DO_BOTH?
P3 - thread for setimask

Best case:  8 cycles
Worst case:  12 cycles + 1 cycle mispredict penalty

*/

	.p2align 2
	.globl H2K_check_sanity_unlock
	.type	H2K_check_sanity_unlock, @function
H2K_check_sanity_unlock:
	{
		R5:4 = memd(H2K_GP + #KG_ready_masks);
		R1 = memw(H2K_GP + #KG_runlist_valids);
		R7.L = #HI(RESCHED_INT_MASK);
		R6 = add(H2K_GP,#KG_runlist); 
	}
	{		
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);  
		R8 = not(R4);		//  ~H2K_ready_validmask
		R10 = and(R5,R4);	//  H2K_ready_valids & H2K_ready_validmask
		R9 = cl0(R1);		//  runlist_worst_prio()
	}
	{
		P0 = cmp.eq(R3,#0);	//  priomask == 0 for lowprio_notify()
		R9 = sub(#MAX_PRIO,R9);	//  runlist_worst_prio()
		R11 = ct0(R10);		//  ready_best_prio()
		P2 = bitsclr(R1,R8);	//  runlist_valids & ~ready_validmask; !P2 really means do them both.
	}
	{
		P0 = or(P0,!P2);	//  if the priomask or runlist check came up zero, skip lowprio_notify
		R1 = addasl(R6,R9,#2);	//  pointer to thread's context; assume 32bit ptr; recycle R1
		if (!P0.new) jump:t 1f;	//  skip lowprio_notify() stuff
		P1 = cmp.gt(R9,R11);	//  IS_WORSE_THAN? first sched check
	}  // 4 cycles
	//  This is mostly lowprio_notify() preparation, with some sched stuff thrown in.
	{
		R1 = memw(R1);		//  load thread_context pointer
		R4 = #0;		//  recycle R4
		P0 = cmp.eq(R2,#0);	//  waitmask for sched, recycle P0
		P1 = cmp.eq(R10,#0);	//  H2K_ready_valids & H2K_ready_validmask; recycle P1
	}  
	{
		R6 = memub(R1+#CONTEXT_hthread);	//  fetch the hthread, recycle R6
		P0 = and(P0,P1);   			//  second sched check
		R7.L = #LO(RESCHED_INT_INTMASK);	//  prepare for sched
	}
	{
		P3 = R6;		//  setimask param
		R3 = setbit(R3,R6);	//  set thread as lowest priority
		P0 = or(P0,!P2);	//  don't forget the P2 check
	}
	{
		setimask(P3,R4);	//  lowprio_notify()
	} 

	{
		memw(H2K_GP+#KG_priomask) = R3; 	//  save new priomask
		if (P0) jump 2f; 			//  do the final sched check
		jump 3f;				//  GTFO
	}
1:	//  if we get here, P2 had to be zero.  Don't check it again.  P1 was pre-loaded with IS_WORSE_THAN
	{	
		if (P1) jump 2f;		//  use previous compare result to see if we need to resched
		R7.L = #LO(RESCHED_INT_MASK);	//  redo the sched prepare that we didn't get a chance to since we skipped lowprio_notify
		P0 = cmp.eq(R2,#0);		//  waitmask == 0, recycle P0
		P1 = cmp.eq(R10,#0);		//  (ready_valids & ready_validmask) == 0, recycle P1
	}  //  HELP:  possible to save a cycle here?  seems kind of lame
	{
		P0 = and(P0,P1);		//  last check, recycle P0
		if (!P0.new) jump:t 3f;		//  still failed, GTFO
	}  //  ~16 bytes of space wasted because I did the second sched compare twice.
	//  Caveat carpe jugulum:  resched_int requires R7 to be set up before we get here.
2:
	swi(R7);
3:
	k0unlock;
	jumpr R31;
	.size	H2K_check_sanity_unlock, .-H2K_check_sanity_unlock

//  crappy cut and paste until we figure out what to do exactly.

	.p2align 2
	.globl H2K_check_sanity
	.type	H2K_check_sanity, @function
H2K_check_sanity:
	{
		R5:4 = memd(H2K_GP + #KG_ready_masks);
		R1 = memw(H2K_GP + #KG_runlist_valids);
		R7.L = #HI(RESCHED_INT_MASK);
		R6 = add(H2K_GP,#KG_runlist); 
	}
	{		
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);  
		R8 = not(R4);		//  ~H2K_ready_validmask
		R10 = and(R5,R4);	//  H2K_ready_valids & H2K_ready_validmask
		R9 = cl0(R1);		//  runlist_worst_prio()
	}
	{
		P0 = cmp.eq(R3,#0);	//  priomask == 0 for lowprio_notify()
		R9 = sub(#MAX_PRIO,R9);	//  runlist_worst_prio()
		R11 = ct0(R10);		//  ready_best_prio()
		P2 = bitsclr(R1,R8);	//  runlist_valids & ~ready_validmask; !P2 really means do them both.
	}
	{
		P0 = or(P0,!P2);	//  if the priomask or runlist check came up zero, skip lowprio_notify
		R1 = addasl(R6,R9,#2);	//  pointer to thread's context; assume 32bit ptr; recycle R1
		if (!P0.new) jump:t 1f;	//  skip lowprio_notify() stuff
		P1 = cmp.gt(R9,R11);	//  IS_WORSE_THAN? first sched check
	}  // 4 cycles
	//  This is mostly lowprio_notify() preparation, with some sched stuff thrown in.
	{
		R1 = memw(R1);		//  load thread_context pointer
		R4 = #0;		//  recycle R4
		P0 = cmp.eq(R2,#0);	//  waitmask for sched, recycle P0
		P1 = cmp.eq(R10,#0);	//  H2K_ready_valids & H2K_ready_validmask; recycle P1
	}  
	{
		R6 = memub(R1+#CONTEXT_hthread);	//  fetch the hthread, recycle R6
		P0 = and(P0,P1);   			//  second sched check
		R7.L = #LO(RESCHED_INT_INTMASK);	//  prepare for sched
	}
	{
		P3 = R6;		//  setimask param
		R3 = setbit(R3,R6);	//  set thread as lowest priority
		P0 = or(P0,!P2);	//  don't forget the P2 check
	}
	{
		setimask(P3,R4);	//  lowprio_notify()
	} 

	{
		memw(H2K_GP+#KG_priomask) = R3; 	//  save new priomask
		if (P0) jump 2f; 			//  do the final sched check
		jump 3f;				//  GTFO
	}
1:	//  if we get here, P2 had to be zero.  Don't check it again.  P1 was pre-loaded with IS_WORSE_THAN
	{	
		if (P1) jump 2f;		//  use previous compare result to see if we need to resched
		R7.L = #LO(RESCHED_INT_MASK);	//  redo the sched prepare that we didn't get a chance to since we skipped lowprio_notify
		P0 = cmp.eq(R2,#0);		//  waitmask == 0, recycle P0
		P1 = cmp.eq(R10,#0);		//  (ready_valids & ready_validmask) == 0, recycle P1
	}  //  HELP:  possible to save a cycle here?  seems kind of lame
	{
		P0 = and(P0,P1);		//  last check, recycle P0
		if (!P0.new) jump:t 3f;		//  still failed, GTFO
	}  //  ~16 bytes of space wasted because I did the second sched compare twice.
	//  Caveat carpe jugulum:  resched_int requires R7 to be set up before we get here.
2:
	swi(R7);
3:
	jumpr R31;
	.size	H2K_check_sanity, .-H2K_check_sanity


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
		R3.H = #HI(RESCHED_INT_INTMASK);
	}
	{
		memw(H2K_GP+#KG_priomask) = R4; //  save priomask
		R3.L = #LO(RESCHED_INT_INTMASK);
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
R6 - H2K_GP->runlist, hthread
R7 - #RESCHED_INT_INTMASK
R8 - ~ready_validmask
R9 - runlist_worst_prio()
R10 - ready_valids & ready_validmask
R11 - ready_best_prio()

P0 - DO_NOTIFY?, DO_SCHED?
P1 - DO_SCHED?
P2 - DO_BOTH?
P3 - thread for setimask

Best case:  8 cycles
Worst case:  13 cycles + 1 cycle mispredict penalty

*/

	.p2align 2
	.globl H2K_check_sanity_unlock
	.type	H2K_check_sanity_unlock, @function
H2K_check_sanity_unlock:
	{
		r5:4 = memd(H2K_GP + #KG_ready_masks);
		r1 = memw(H2K_GP + #KG_runlist_valids);
		r7.l = #hi(RESCHED_INT_INTMASK);
		r6 = add(H2K_GP,#KG_runlist); 
	}
	{		
		r3:2 = memd(H2K_GP + #KG_lowprio_masks);  
		r8 = not(r4);		//  ~H2K_ready_validmask
		r10 = and(r5,r4);	//  H2K_ready_valids & H2K_ready_validmask
		r9 = cl0(r1);		//  runlist_worst_prio()
	}
	{
		p0 = cmp.eq(r3,#0);	//  priomask == 0 for lowprio_notify()
		r9 = sub(#MAX_PRIO,r9);	//  runlist_worst_prio()
		r11 = ct0(r10);		//  ready_best_prio()
		p2 = bitsclr(r1,r8);	//  runlist_valids & ~ready_validmask; !p2 really means do them both.
	}
	{
		p0 = or(p0,!p2);	//  if the priomask or runlist check came up zero, skip lowprio_notify
		if (!p0.new) jump:t 1f;	//  skip lowprio_notify() stuff
		p1 = cmp.gt(r9,r11);	//  is_worse_than? first sched check
		p3 = cmp.eq(r2,#0);     //  start second check (waitmask)
	}  //  4 cycles
	//  this is mostly lowprio_notify() preparation, with some sched stuff thrown in.
	{
		r1 = addasl(r6,r9,#2);	//  pointer to thread's context; assume 32bit ptr; recycle r1
		p0 = cmp.eq(r12,#0);	//  second sched check (ready valids), recycle p0
		p3 = not(p3);		//  waitmask !=  0
	} 
	{
		r1 = memw(r1);		//  load thread_context pointer
		r4 = #0;		//  recycle r4
		p0 = and(p3,!p0);	//  if waitmask != 0 && ready_valids != 0
	}  
	{
		r6 = memub(r1+#CONTEXT_hthread);	//  fetch the hthread, recycle r6
		r7.l = #lo(RESCHED_INT_INTMASK);	//  prepare for sched
		p0 = or(p0,!p2);			//  don't forget the p2 check
	}
	{
		p3 = r6;		//  setimask param
		r3 = setbit(r3,r6);	//  set thread as lowest priority
	}
	{
		setimask(p3,r4);	//  lowprio_notify()
	} 

	{
		memw(H2K_GP+#KG_priomask) = r3; 	//  save new priomask
		if (p0) jump 2f; 			//  do the final sched check
		jump 3f;				//  gtfo
	}
1:	//  if we get here, p2 had to be zero.  don't check it again.  p1 was pre-loaded with is_worse_than
	{	
		if (p1) jump 2f;		//  use previous compare result to see if we need to resched
		r7.l = #lo(RESCHED_INT_INTMASK);	//  redo the sched prepare that we didn't get a chance to since we skipped lowprio_notify
		p3 = not(p3);			//  waitmask != 0
		p1 = cmp.eq(r10,#0);		//  (ready_valids & ready_validmask) == 0, recycle p1
	}  //  help:  possible to save a cycle here?  seems kind of lame
	{
		p0 = and(p3,!p1);		//  last check, recycle p0
		if (!p0.new) jump:t 3f;		//  still failed, gtfo
	}  //  ~16 bytes of space wasted because i did the second sched compare twice.
	//  caveat carpe jugulum:  resched_int requires r7 to be set up before we get here.
2:
	swi(r7);
3:
	k0unlock;
	jumpr r31;
	.size	H2K_check_sanity_unlock, .-H2K_check_sanity_unlock

//  crappy cut and paste until we figure out what to do exactly.

	.p2align 2
	.globl H2K_check_sanity
	.type	H2K_check_sanity, @function
H2K_check_sanity:
	{
		r5:4 = memd(H2K_GP + #KG_ready_masks);
		r1 = memw(H2K_GP + #KG_runlist_valids);
		r7.l = #hi(RESCHED_INT_INTMASK);
		r6 = add(H2K_GP,#KG_runlist); 
	}
	{		
		r3:2 = memd(H2K_GP + #KG_lowprio_masks);  
		r8 = not(r4);		//  ~H2K_ready_validmask
		r10 = and(r5,r4);	//  H2K_ready_valids & H2K_ready_validmask
		r9 = cl0(r1);		//  runlist_worst_prio()
	}
	{
		p0 = cmp.eq(r3,#0);	//  priomask == 0 for lowprio_notify()
		r9 = sub(#MAX_PRIO,r9);	//  runlist_worst_prio()
		r11 = ct0(r10);		//  ready_best_prio()
		p2 = bitsclr(r1,r8);	//  runlist_valids & ~ready_validmask; !p2 really means do them both.
	}
	{
		p0 = or(p0,!p2);	//  if the priomask or runlist check came up zero, skip lowprio_notify
		if (!p0.new) jump:t 1f;	//  skip lowprio_notify() stuff
		p1 = cmp.gt(r9,r11);	//  is_worse_than? first sched check
		p3 = cmp.eq(r2,#0);     //  start second check (waitmask)
	}  //  4 cycles
	//  this is mostly lowprio_notify() preparation, with some sched stuff thrown in.
	{
		r1 = addasl(r6,r9,#2);	//  pointer to thread's context; assume 32bit ptr; recycle r1
		p0 = cmp.eq(r12,#0);	//  second sched check (ready valids), recycle p0
		p3 = not(p3);		//  waitmask !=  0
	} 
	{
		r1 = memw(r1);		//  load thread_context pointer
		r4 = #0;		//  recycle r4
		p0 = and(p3,!p0);	//  if waitmask != 0 && ready_valids != 0
	}  
	{
		r6 = memub(r1+#CONTEXT_hthread);	//  fetch the hthread, recycle r6
		r7.l = #lo(RESCHED_INT_INTMASK);	//  prepare for sched
		p0 = or(p0,!p2);			//  don't forget the p2 check
	}
	{
		p3 = r6;		//  setimask param
		r3 = setbit(r3,r6);	//  set thread as lowest priority
	}
	{
		setimask(p3,r4);	//  lowprio_notify()
	} 

	{
		memw(H2K_GP+#KG_priomask) = r3; 	//  save new priomask
		if (p0) jump 2f; 			//  do the final sched check
		jump 3f;				//  gtfo
	}
1:	//  if we get here, p2 had to be zero.  don't check it again.  p1 was pre-loaded with is_worse_than
	{	
		if (p1) jump 2f;		//  use previous compare result to see if we need to resched
		r7.l = #lo(RESCHED_INT_INTMASK);	//  redo the sched prepare that we didn't get a chance to since we skipped lowprio_notify
		p3 = not(p3);			//  waitmask != 0
		p1 = cmp.eq(r10,#0);		//  (ready_valids & ready_validmask) == 0, recycle p1
	}  //  help:  possible to save a cycle here?  seems kind of lame
	{
		p0 = and(p3,!p1);		//  last check, recycle p0
		if (!p0.new) jump:t 3f;		//  still failed, gtfo
	}  //  ~16 bytes of space wasted because i did the second sched compare twice.
	//  caveat carpe jugulum:  resched_int requires r7 to be set up before we get here.
2:
	swi(r7);
3:
	jumpr r31;
	.size	H2K_check_sanity, .-H2K_check_sanity


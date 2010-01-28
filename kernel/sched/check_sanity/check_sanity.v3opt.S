/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.text
	.p2align 2
	.globl H2K_check_sched_mask
	.type	H2K_check_sched_mask, @function
H2K_check_sched_mask:
	// saved LR + FP regs size (bytes) = 0
	// callee saved regs size (bytes) = 0
	// local vars size (bytes) = 0
	// fixed args size (bytes) = 0
	{
		r2 = memw(r16+#KG_runlist_valids)
		r1 = memw(r16+#KG_ready_validmask)
		r3 = #0
	}
	{
		r0 = cl0(r2)
		r1 = not(r1)
	}
	{
		r0 = sub(#MAX_PRIO,r0)
		p0 = bitsclr(r2,r1)
	}
	{
		r0 = addasl(r16,r0,#2)
		if (p0) jump .L1
	}
	{
		r1 = memw(r0+#KG_runlist)
		r0 = memw(r16+#KG_priomask)
	}
		r1 = memub(r1+#CONTEXT_hthread)
		r0 = setbit(r0,r1)
		memw(r16+#KG_priomask) = r0
//APP
		 p0 = r1
	 setimask(p0,r3)
//NO_APP
		r0.h = #HI(RESCHED_INT_MASK)
		r0.l = #LO(RESCHED_INT_MASK)
//APP
		 swi(r0) // wake up & resched
//NO_APP
.L1:
		jumpr r31
	.size	H2K_check_sched_mask, .-H2K_check_sched_mask

/*
H2K_check_sanity, v3 optimized

Notes:

Registers used:

R1 - runlist_valids, thread_context pointer
R2 - waitmask
R3 - priomask
R4 - ready_validmask, imask param
R5 - ready_valids
R6 - H2K_gp->runlist
R7 - #RESCHED_INT_MASK
R8 - ~ready_validmask
R9 - runlist_worst_prio()
R10 - ready_valids & ready_validmask
R11 - ready_best_prio()

P0 - DO_NOTIFY?, DO_SCHED?
P1 - DO_SCHED?
P2 - DO_BOTH?
P3 - thread for setimask

Best case:  7 cycles
Worst case:  11 cycles + 1 cycle mispredict penalty

*/

	.p2align 2
	.globl H2K_check_sanity
	.type	H2K_check_sanity, @function
H2K_check_sanity:

#define H2K_GP r16

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
		R1 = addasl(R6,R9,#2);	//  pointer to thread's context; recycle R1
		if (!P0.new) jump:t 1f;	//  SKIP_NOTIFY
		P1 = cmp.gt(R9,R11);	//  IS_WORSE_THAN? first sched check
	}  // 4 cycles
	//  This is mostly lowprio_notify() preparation, with some sched stuff thrown in.
	{
		R1 = memw(R1);		//  load thread_context pointer
		R4 = #0;		//  recycle R4
		P0 = cmp.eq(R2,#0);	//  waitmask for sched, recycle P0
		P1 = cmp.eq(R10,#0);	//  H2K_ready_valids & H2K_ready_validmask
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
		if (P2) jump 2f; 			//  if check_sched_mask was true, we already have our scheduler decision
		jump 3f;				//  GTFO
	}
1:	//  if we get here, P2 had to be zero.  Don't check it again.  P1 was pre-loaded with IS_WORSE_THAN
	{	
		if (P1) jump 2f;		//  use previous compare result to see if we need to resched
		R7.L = #LO(RESCHED_INT_MASK);	//  redo the sched prepare that we didn't get a chance to since we skipped lowprio_notify
		P0 = cmp.eq(R2,#0);		//  waitmask == 0, recycle P0
		P1 = cmp.eq(R10,#0);		//  (ready_valids & ready_validmask) == 0, recycle P1
	}  //  possible to save a cycle here?  this seems kind of lame
	{
		P0 = and(P0,P1);		//  last check, recycle P0
		if (!P0.new) jump:t 3f;		//  still failed, GTFO
	}
	//  Caveat carpe jugulum:  resched_int requires R7 to be set up before we get here.
2:
	swi(R7);
3:
	jumpr R31;
	.size	H2K_check_sanity, .-H2K_check_sanity

	.section	.rodata.str1.8,"aMS",@progbits,1
	.p2align 3
.LC0:
	.string	"include/check_sanity_debug.h:6 checker_kernel_locked()"
	.p2align 3
.LC1:
	.string	"include/check_sanity_debug.h:8 checker_kernel_locked()"
	.p2align 3
.LC2:
	.string	"include/check_sanity_debug.h:9 retval == arg0"
	.text
	.p2align 2
	.globl H2K_check_sanity_unlock
	.type	H2K_check_sanity_unlock, @function
H2K_check_sanity_unlock:
	// saved LR + FP regs size (bytes) = 8
	// callee saved regs size (bytes) = 16
	// local vars size (bytes) = 0
	// fixed args size (bytes) = 0
	{
		allocframe(#16)
		call __save_r27_through_r24
	}
	{
		r27:26 = r1:0
		call checker_kernel_locked
	}
	{
		r1 = #0
		if (r0!=#0) jump:t .L21
	}
		r0.h = #HI(.LC0)
	{
		r0.l = #LO(.LC0)
		call _Assert
	}
.L21:
	{
		r1:0 = r27:26
		call H2K_check_sanity
	}
	{
		r25:24 = r1:0
		call checker_kernel_locked
	}
	{
		r1 = #0
		if (r0!=#0) jump:t .L23
	}
		r0.h = #HI(.LC1)
	{
		r0.l = #LO(.LC1)
		call _Assert
	}
.L23:
	{
		r0.h = #HI(.LC2)
		p0 = cmp.eq(r25:24,r27:26)
		r1 = #0
	}
	{
		r0.l = #LO(.LC2)
		if (p0) jump .L25
	}
		call _Assert
.L25:
//APP
	 k0unlock
//NO_APP
	{
		r1:0 = r27:26
		jump __restore_r27_through_r24_and_deallocframe
	}
	.size	H2K_check_sanity_unlock, .-H2K_check_sanity_unlock
	.ident	"GCC: (GNU) 3.4.6 Hexagon Build Version (1.2) BT_200911070024_lnx64"

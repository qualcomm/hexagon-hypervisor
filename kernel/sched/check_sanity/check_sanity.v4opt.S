/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <max.h>

FUNC_START H2K_check_sched_mask .text.core.sanity .p2align 5

//  3 cycles best case, 9 worse + 2? mispredict
	{
		R2 = memw(H2K_GP+#KG_runlist_valids);
		R1 = memw(H2K_GP+#KG_ready_validmask);
		R3 = #0;
	}
	{
		R0 = and(R2,~R1);	//  runlist_valids & ~ready_validmask
		R1 = cl0(R2);		//  runlist_worst_prio
		if (cmp.eq(R0.new,#0)) jump:t 1f;
	} 
	{
		R1 = sub(#MAX_PRIO,R1);		//  runlist_worst_prio
		R2 = add(H2K_GP,#KG_runlist);
		R5 = memw(H2K_GP+#KG_priomask);
	}
	{
		R0 = memw(R2+R1<<#2);  //  get pointer to thread_context
		R4 = ##RESCHED_INT_INTMASK;
	}  
	{
		R1 = memub(R0+#CONTEXT_hthread);
	}
	{
		P0 = R1;
		R5 = setbit(R5,R1);
		memw(H2K_GP+#KG_priomask) = R5.new;
	}
	{
		setimask(P0,R3);
	}
	{
		swi(R4);  //  could save a register if swi could be reordered
	}

1:
	{
		jumpr R31;
	}
FUNC_END H2K_check_sched_mask

/*
H2K_check_sanity, v4 optimized

R0 - do not touch.
R1 - runlist_valids
R2 - priomask
R3 - waitmask
R4 - ready_valids, hthread
R5 - ready_validmask
R6 - check_sched_mask result
R7 - runlist_worst_prio()
R8 - ready_best_prio()
R9 - ready_valids & validmask
R10 - RESCHED_INT_INTMASK

P0 - DO_SCHED?
P1 - DO_NOTIFY?
P3 - thread for setimask

Best case:  7 cycles
Worst case:  12 cycles + 2 cycle mispredict penalty (1 cycle lost to assembler bug)

*/

FUNC_START H2K_check_sanity_unlock .text.core.sanity .p2align 5
	{
		R5:4 = memd(H2K_GP + #KG_ready_masks);  
		R1 = memw(H2K_GP + #KG_runlist_valids); 
		R11 = add(H2K_GP,#KG_runlist);		//  prepare loprio_notify
		R10.H = #HI(RESCHED_INT_INTMASK);	//  prepare resched_int
	}
	{
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);
		R6 = and(R1,~R5); 	//  runlist_valids &~ready_validmask
		R7 = cl0(R1);		//  runlist_worst_prio()
		R9 = and(R4,R5);	//  ready_valids & ready_validmask
	}
	{
		R7 = sub(#MAX_PRIO,R7);			//  runlist_worst_prio()
		P0 = !cmp.eq(R2,#0);			//  Duplex - priomask == 0?
		P0 = cmp.eq(R6,#0);			//  check_sched_mask?
		if (P0.new) jump:t 3f;			//  whoah this is one packet!
		//R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int  ASSEMBLER BUG?
	}  //  note:  retain R6 for subsequent compares in lowprio_notify
	//  do lowprio_notify
	{
		R4 = memw(R11+R7<<#2);	//  load hthread; recycle R4
		R8 = ct0(R9);			//  ready_best_prio()
		P0 = cmp.eq(R3,#0);		//  D - wait_mask
		P1 = cmp.eq(R9,#0);		//  ready_validmasks
	}
	{
		R4 = memub(R4 + #CONTEXT_hthread);
	}  // argh!
	{
		R2 = setbit(R2,R4);			//  set new priomask
		memw(H2K_GP+#KG_priomask) = R2.new;	//  set new priomask
		P3 = R4;				//  prepare setimask
		R5 = #0;				//  D - 0 prepare setimask
	}
	{
		setimask(P3,R5);  //  set register somewhere
	}
	{
		P0 = or(P0,P1);		//  building a reason to bail out
		P0 = cmp.eq(R6,#0);	//  D - check  R6 is check_sched_mask compare; another reason to bail out
		P0 = !cmp.gt(R7,R8);	//  last reason to bail out
		if (P0.new) jump:t 2f;
	}  //  just did like 80 bajillion comparisons in one packet

	R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int  ASSEMBLER BUG?
1:
	swi(R10);
2:
	k0unlock;
	jumpr r31;

3:  //  R6 was already cmpared, so don't check it again
	{
		R8 = ct0(R9);		//  ready_best_prio()
		P0 = cmp.eq(R3,#0);	//  wait_mask
		P1 = cmp.eq(R9,#0);	//  ready_validmasks
		R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int
	}  //  only need to check the two 'if' clauses now
	{
		P0 = !cmp.gt(R7,R8);	//  is_worse_than?
		P0 = or(P0,P1);
		if (!P0.new) jump:t 1b;  //  this should be a single compare-jump instruction, not that it matters.
	}
	k0unlock;
	jumpr r31;
FUNC_END H2K_check_sanity_unlock 

//  crappy cut and paste until we figure out what to do exactly.

FUNC_START H2K_check_sanity .text.core.sanity .p2align 5
	{
		R5:4 = memd(H2K_GP + #KG_ready_masks);  
		R1 = memw(H2K_GP + #KG_runlist_valids); 
		R11 = add(H2K_GP,#KG_runlist);		//  prepare loprio_notify
		R10.H = #HI(RESCHED_INT_INTMASK);	//  prepare resched_int
	}
	{
		R3:2 = memd(H2K_GP + #KG_lowprio_masks);
		R6 = and(R1,~R5); 	//  runlist_valids &~ready_validmask
		R7 = cl0(R1);		//  runlist_worst_prio()
		R9 = and(R4,R5);	//  ready_valids & ready_validmask
	}
	{
		R7 = sub(#MAX_PRIO,R7);			//  runlist_worst_prio()
		P0 = !cmp.eq(R2,#0);			//  Duplex - priomask == 0?
		P0 = cmp.eq(R6,#0);			//  check_sched_mask?
		if (P0.new) jump:t 3f; 			//  whoah this is one packet!
		//R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int  ASSEMBLER BUG?
	}  //  note:  retain R6 for subsequent compares in lowprio_notify
	//  do lowprio_notify
	{
		R4 = memw(R11+R7<<#2);		//  load hthread; recycle R4
		R8 = ct0(R9);			//  ready_best_prio()
		P0 = cmp.eq(R3,#0);		//  D - wait_mask
		P1 = cmp.eq(R9,#0);		//  ready_validmasks
	}
	{
		R4 = memub(R4 + #CONTEXT_hthread);
	}  // argh!
	{
		R2 = setbit(R2,R4);			//  set new priomask
		memw(H2K_GP+#KG_priomask) = R2.new;	//  set new priomask
		P3 = R4;				//  prepare setimask
		R5 = #0;			//  D - 0 prepare setimask
	}
	{
		setimask(P3,R5);  //  set register somewhere
	}
	{
		P0 = or(P0,P1);		//  building a reason to bail out
		P0 = cmp.eq(R6,#0);	//  D - check  R6 is check_sched_mask compare; another reason to bail out
		P0 = !cmp.gt(R7,R8);	//  last reason to bail out
		if (P0.new) jump:t 2f;  
	}  //  just did like 80 bajillion comparisons in one packet

	R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int  ASSEMBLER BUG?
1:
	swi(R10);
2:
	jumpr r31;

3:  //  R6 was already cmpared, so don't check it again
	{
		R8 = ct0(R9);		//  ready_best_prio()
		P0 = cmp.eq(R3,#0);	//  wait_mask
		P1 = cmp.eq(R9,#0);	//  ready_validmasks
		R10.L = #LO(RESCHED_INT_INTMASK);	//  prepare resched_int
	}  //  only need to check the two 'if' clauses now
	{
		P0 = !cmp.gt(R7,R8);	//  is_worse_than?
		P0 = or(P0,P1);
		if (!P0.new) jump:t 1b;  //  this should be a single compare-jump instruction, not that it matters.
	}
	jumpr r31;
FUNC_END H2K_check_sanity


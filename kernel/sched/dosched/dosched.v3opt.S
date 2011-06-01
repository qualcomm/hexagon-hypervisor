/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* dosched.v3opt.S
 * This contains fast implementation of dosched.c function
 * for V3
 */

#include <asm_std.h>
#include <asm_offsets.h>

/*------------------------------------------------------------------*/
/* H2K_dosched(H2K_thread_context *me, u32_t hthread)               */
/*------------------------------------------------------------------*/
/* Register Usage: R0-R6, R17-R21                                   */
/*------------------------------------------------------------------*/

FUNC_START H2K_dosched  .text.core.dosched  .p2align  3
	/*--------------------------------------------------*/
	/* new = H2K_ready_getbest()                        */
	/*--------------------------------------------------*/
	{
		r19:18 = r1:0                                   // save hthread : me in regs
		r1:0 = memd(H2K_GP+#KG_ready_valids)            // H2K_gp->ready_valids[0]
		r3:2 = memd(H2K_GP+#KG_ready_valids + 8)        // H2K_gp->ready_valids[1]
		r9:8 = #0
	}
	{
		p0 = cmp.eq(r1:0,r9:8)
		p1 = cmp.eq(r3:2,r9:8)
		r5:4 = memd(H2K_GP+#KG_ready_valids + 16)       // H2K_gp->ready_valids[2]
		r7:6 = memd(H2K_GP+#KG_ready_valids + 24)       // H2K_gp->ready_valids[3]
	}
	{
		p2 = cmp.eq(r5:4,r9:8)
		p3 = cmp.eq(r7:6,r9:8)
	}
	{
		p0 = and(p0,p1)
		p1 = and(p2,p3)
	}
	{
		r17 = #0                                        // new = NULL
		p0 = and(p0,p1)
		if (p0.new) jump:t .H2K_dosched_ready_getbest_done
		                                                // if (!H2K_ready_any_valid()) done with H2K_ready_getbest()
	}
	{
		r0 = ct0(r0)
	}
	{
		p0 = cmp.gt(r0,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r1)
	}
	{
		r0 = add(r0,r1)
		p0 = cmp.gt(r1,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r2)
	}
	{
		r0 = add(r0,r1)
		p0 = cmp.gt(r1,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r3)
	}
	{
		r0 = add(r0,r1)
		p0 = cmp.gt(r1,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r4)
	}
	{
		r0 = add(r0,r1)
		p0 = cmp.gt(r1,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r5)
	}
	{
		r0 = add(r0,r1)
		p0 = cmp.gt(r1,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r6)
	}
	{
		r0 = add(r0,r1)
		p0 = cmp.gt(r1,#31)
		if (!p0.new) jump:t .H2K_dosched_ready_best_prio_done
		r1 = ct0(r7)
	}
	{
		r0 = add(r0,r1)
	}
	.falign
.H2K_dosched_ready_best_prio_done:
	{
		r0 = addasl(H2K_GP,r0,#2)                       // for &H2K_gp->ready[prio]
	}
	{
		r17 = memw(r0+#KG_ready)                        // new = H2K_gp->ready[prio]
	}
	{
		r21 = memub(r17+#CONTEXT_prio)                  // new->prio
		r0 = add(H2K_GP,#KG_ready)                      // &H2K_gp->ready[0]
	}
	{
		r0 += asl(r21,#2)                               // func call: r0= &H2K_gp->ready[prio]
		r1 = r17                                        // func call: r1= new
		call H2K_ring_remove_real                       // H2K_ring_remove()
	}
	{
		r0 = addasl(H2K_GP,r21,#2)                      // for &H2K_gp->ready[prio]
	}
	{
		r0 = memw(r0+#KG_ready)                         // H2K_gp->ready[prio]
		r1 = add(H2K_GP,#KG_ready_valids)               // &H2K_gp->ready_valids[0]
	}
	{
		r1 = tableidxw(r21,#3,#5)                       // &((u32_t) H2K_gp->ready_valids)[prio >> 5]
		r6 = and(r21,#0x1f)                             // prio & 0x1f
		if (r0!=#0) jump:t .H2K_dosched_ready_getbest_done
		                                                // if (H2K_gp->ready[prio]!=NULL) jump
	}
	{
		r0 = memw(r1)                                   // to update H2K_gp->ready_valids
	}
	{
		r0 = clrbit(r0,r6)                              // clrbit(H2K_gp->ready_valids,prio)
	}
	{
		memw(r1) = r0                                   // clrbit(H2K_gp->ready_valids,prio)
	}
	.falign
.H2K_dosched_ready_getbest_done:
	{
		r1 = memw(H2K_GP+#KG_wait_mask)                 // H2K_gp->wait_mask
		p0 = cmp.eq(r17,#0)                             // if(new==NULL)
		if ( p0.new) jump:nt .H2K_dosched_newthread_null// then jump
	}
	{
		r2 = memw(H2K_GP+#KG_priomask)                  // H2K_gp->priomask
		p0 = cmp.eq(r1,#0)                              // P0 = (H2K_gp->wait_mask==0)
		if (p0.new) r9:8 = memd(H2K_GP + #KG_runlist_prios)
                                                                // H2K_gp->runlist_prios[0-3]
	}
	{
		r6 = #STATUS_RUNNING                            //  H2K_STATUS_RUNNING
		p1 = tstbit(r2,r19)                             // P1 = (H2K_gp->priomask&(1<<hthread)!=0)
		if (!p0) jump .H2K_dosched_threads_waiting_or_high_prio
		                                                // if !(H2K_gp->wait_mask==0) then jump
		if (p0) r11:10 = memd(H2K_GP + #KG_runlist_prios + 8)
                                                                // H2K_gp->runlist_prios[4-7]
	}
	{
		r9:8 = vmaxh(r11:10,r9:8)
	}
	{
		r8 = sxth(r8)
		r9 = asrh(r8)
		r10 = sxth(r9)
		r11 = asrh(r9)
	}
	{
		p0 = cmp.gt(r9,r8)
		if (p0.new) r8 = r9                             // max(H2K_gp->runlist_prios[0-1,4-5])
	}
	{
		p0 = cmp.gt(r10,r8)
		if (p0.new) r8 = r10                            // max(H2K_gp->runlist_prios[0-2,4-6])
	}
	{
		p0 = cmp.gt(r11,r8)
		if (p0.new) r8 = r11                            // max(H2K_gp->runlist_prios[0-7])
	}
	{
		p0 = cmp.gt(r21,r8)                             // (new->prio IS_WORSE_THAN H2K_runlist_worst_prio())
		if (!p0.new) jump:nt .H2K_dosched_threads_waiting_or_high_prio
		                                                //  if !p0 then jump
	}
	/*--------------------------------------------------*/
	/* if((H2K_gp->wait_mask==0) && (new->prio          */
	/*       IS_WORSE_THAN H2K_runlist_worst_prio()))   */
	/*--------------------------------------------------*/
	{
		if ( p1) jump .H2K_dosched_runlist_push         // if !((H2K_gp->priomask & (1<<hthread)) ==0) continue
		if (!p1) r3 = #1                                // 1
		r0 = ct0(r2)                                    // ct0(H2K_gp->priomask)
	}
	{
		p0 = r0                                         // H2K_raise_lowprio() : change_imask(ct0(H2K_gp->priomask),-1)
		r1:0 = combine(#-1,#0)                          // r1 -- H2K_raise_lowprio(): change_imask(,-1)
		                                                // r0 -- to set low priority
		r3 = asl(r3,r19)                                // 1<<hthread
	}
	{
		setimask(p0,r1)                                 // H2K_raise_lowprio() : change_imask(ct0(H2K_gp->priomask),-1)
	}
	{
		imask = r0                                      // set to low priority
	}
	{
		memw(H2K_GP+#KG_priomask) = r3                  // H2K_gp->priomask |= 1<<thread
		jump .H2K_dosched_runlist_push
	}

	/*--------------------------------------------------*/
	/* else                                             */
	/*--------------------------------------------------*/
	.falign
.H2K_dosched_threads_waiting_or_high_prio:
	{
		if (!p1) jump .H2K_dosched_runlist_push         // if !((H2K_gp->priomask & (1<<hthread))!=0) continue
		r3=clrbit(r2,r19)                               // clrbit(H2K_gp->priomask,hthread)
		NOP; NOP
	}
	{
		memw(H2K_GP+#KG_priomask) = r3                  // H2K_gp->priomask=clrbit(H2K_gp->priomask,hthread)
		r0 = #-1                                        // to set high priority
		NOP
	}
	{
		imask = r0                                      // set to high priority
	}
	.falign
.H2K_dosched_runlist_push:
	/*--------------------------------------------------*/
	/* H2K_runlist_push(new)                            */
	/*--------------------------------------------------*/
	{
		memb(r17+#CONTEXT_status) = r6                  // new->status = H2K_STATUS_RUNNING
		r2 = addasl(H2K_GP,r19,#2)                      // for &H2K_gp->runlist[hthread]
		r3 = addasl(H2K_GP,r19,#1)                      // for &H2K_gp->runlist_prios[hthread]
	}
	{
		memw(r2+#KG_runlist) = r17                      // H2K_gp->runlist[hthread] = new
	}
	{
		memh(r3+#KG_runlist_prios) = r21                // H2K_gp->runlist_prios[hthread] = new->prio
		r1:0 = combine(r17,r18)                         // func call: r0 = me, r1 = new
		call H2K_switch                                 // H2K_switch(me, new)
	}
	{
		// jumpr r31
	}
	.falign
.H2K_dosched_newthread_null:
	{
		p0 = cmp.eq(r1,#0)                              // if(H2K_gp->wait_mask) jump
		if (!p0.new) jump:nt .H2K_dosched_newthread_null_cont
		if ( p0.new) r2 = memw(H2K_GP+#KG_priomask)     // else H2K_gp->priomask
	}
	{
		memw(H2K_GP+#KG_priomask) = r17                 // H2K_gp->priomask = 0
		r0 = ct0(r2)                                    // ct0(mask)
		r1 = #-1                                        // change_imask(ct0(mask),-1)
	}
	{
		p0 = r0                                         // change_imask(ct0(mask),-1)
	}
	{
		setimask(p0,r1)                                 // change_imask(ct0(mask),-1)
	}
	.falign
.H2K_dosched_newthread_null_cont:
	{
		r1:0 = combine(r17,r18)                         // func call: r0 = me, r1 = new
		call H2K_switch                                 // H2K_switch(me, new)
	}
	{
		// jumpr r31
	}
FUNC_END H2K_dosched

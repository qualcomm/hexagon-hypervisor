/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* dosched.v4opt.S
 * This contains fast implementation of dosched.c function
 * for V4
 */

/*
 * Note: This function does not return.  Always calls switch(), which doesn't
 * return
 */

#include <asm_std.h>
#include <asm_offsets.h>

/*------------------------------------------------------------------*/
/* H2K_dosched(H2K_thread_context *me, u32_t hthread)               */
/*------------------------------------------------------------------*/
/* Register Usage: R0-R19, r31                                      */
/*------------------------------------------------------------------*/

FUNC_START H2K_dosched  .text.core.dosched  .p2align  3
	/*--------------------------------------------------*/
	/* new = H2K_ready_getbest()                        */
	/*--------------------------------------------------*/
	{
		r17 = #0                                        // set new =NULL
		r19:18 = r1:0                                   // save hthread : me in r19:18
		r1:0 = memd(H2K_GP+#KG_ready_valids)            // H2K_gp->ready_valids[0]
		r3:2 = memd(H2K_GP+#KG_ready_valids + 8)        // H2K_gp->ready_valids[1]
	}
	{
		r5:4 = memd(H2K_GP+#KG_ready_valids + 16)       // H2K_gp->ready_valids[2]
		r7:6 = memd(H2K_GP+#KG_ready_valids + 24)       // H2K_gp->ready_valids[3]
		r0 = ct0(r1:0)
		r1 = ct0(r3:2)
	}
	{
		r2 = ct0(r5:4)
		r3 = ct0(r7:6)
		p0 = cmp.eq(r0,#64)
		if (p0.new) r0 = add(r0,r1)
	}
	{
		p0 = cmp.eq(r0,#128)
		if (p0.new) r0 = add(r0,r2)
		r3 += add(r0,r2)
		p1 = cmp.eq(r2,#64)
	}
	{
		p1 = and(p0,p1)
		if (p1.new) r0 = r3
		p0 = cmp.eq(r3,#256)
		if (p0.new) jump:nt .H2K_dosched_newthread_null
		                                                // if (r0==0) done with H2K_ready_getbest()
	}

	/* There is a new thread to schedule */
	{
		r10 = add(H2K_GP,#KG_ready)                     // &H2K_gp->ready[0]
		r11 = add(H2K_GP,#KG_ready_valids)              // &H2K_gp->ready_valids[0]
		r9:8 = bitsplit(r0,#5)
	}
	{
		r17 = memw(r10+r0<<#2)                          // new = H2K_gp->ready[prio]
		// tableidx/and to bitsplit?
		//r11 = tableidxw(r0,#3,#5)                       // &((u32_t *) H2K_gp->ready_valids)[prio >> 5]
		//r14 = and(r0,#0x1f)                             // prio & 0x1f
		r5 = memw(r11+r9<<#2)				// H2K_gp->ready_valids[prio>>5]
	}
	{
		r15 = memub(r17+#CONTEXT_prio)                  // prio = new->prio
		r3:2 = memd(r17)				// ring remove: prev:next
		r13:12 = combine(#0,#-1)                        // 0, 0xFFFFFFFF
		r5 = clrbit(r5,r8)				// clear prio bitmask
	}
	{
		r4 = memw(r10+r15<<#2)				// ring remove H2K_gp->ready[prio]
		r1 = r17					// func call: r1 = new
	}
#define NEXT 0
#define PREV 4
	{
		memw(r3+#NEXT) = r2 				// ring remove: prev->next = next
		memw(r2+#PREV) = r3 				// ring remove: next->prev = prev
		p0 = cmp.eq(r4,r1)				// ring remove: removing head?
		p1 = cmp.eq(r1,r2)				// ring remove: node == node->next (only thing in list?)
	}
	{
		r2 = mux(p1,#0,r2)				// ring remove: choose whether next or NULL
		if (p0) memw(r10+r15<<#2) = r2.new		// ring remove: If head removed, update
		p0 = and(p0,p1)					// list is empty?
		r1 = memw(H2K_GP+#KG_wait_mask)                 // H2K_gp->wait_mask
	}
	{
		if (p0) memw(r11+r9<<#2) = r5			// if list is empty, store new mask
		r2 = memw(H2K_GP+#KG_priomask)                  // H2K_gp->priomask
		r14 = #4
	}
	{
		p0 = cmp.eq(r1,#0)                              // P0 = (H2K_gp->wait_mask ==0)
		p1 = tstbit(r2,r19)                             // P1 = (H2K_gp->priomask &(1<<hthread) !=0 )
		r3 = lsl(#1,r19)                                // 1<<hthread
		if (p0.new) r11:10 = memd(H2K_GP + #KG_runlist_prios)
		                                                // H2K_gp->runlist_prios[0-3]
	}
	{
		r6 = add(H2K_GP, #KG_runlist)                   // &H2K_gp->runlist[0]... can be moved down
		if (!p0) jump .H2K_dosched_threads_waiting_or_high_prio
		r13:12 = vrmaxh(r11:10,r13)                     // max(H2K_gp->runlist_prios[0-3])
		if (p0) r11:10 = memd(H2K_GP + #KG_runlist_prios + 8)
		                                                // H2K_gp->runlist_prios[4-7]
	}
	{
		r13:12 = vrmaxh(r11:10,r14)                     // max(H2K_gp->runlist_prios[0-7])
		r1 = ct0(r2)                                    // H2K_raise_lowprio() : ct0(H2K_gp->priomask)
		r5:4 = combine(#-1,#0)                          // r1 -- H2K_raise_lowprio(): change_imask(,-1)
	}
	{
		p0 = cmp.gt(r15,r12)
		if (!p0.new) jump:nt .H2K_dosched_threads_waiting_or_high_prio
		if (p1) jump .H2K_dosched_runlist_push          // if(H2K_gp->priomask &(1<<hthread) !=0) continue
		                                                // if !( new->prio IS_WORSE_THAN H2K_runlist_worst_prio())
		                                                // then jump to else branch

	/*--------------------------------------------------*/
	/* if((H2K_gp->wait_mask==0) && (new->prio          */
	/*       IS_WORSE_THAN H2K_runlist_worst_prio()))   */
	/*--------------------------------------------------*/
	}
	p2 = r1						// H2K_raise_lowprio(): change_imask(ct0(H2K_gp->priomask),-1)
	{
		setimask(p2,r5)                                 // H2K_raise_lowprio(): change_imask(ct0(mask),-1)
	}
	imask = r4                                      // set to low priority
	{
		memw(H2K_GP+#KG_priomask) = r3                  // H2K_gp->priomask |= 1<<hthread, (previous = 0)
		jump .H2K_dosched_runlist_push
	}

	/*--------------------------------------------------*/
	/* else                                             */
	/*--------------------------------------------------*/
	.falign
.H2K_dosched_threads_waiting_or_high_prio:
	{
		// if (!p1) jump .H2K_dosched_runlist_push         // if(H2K_gp->priomask &(1<<hthread) ==0) continue
		r3 = and(r2,~r3)                                // clrbit(H2K_gp->priomask,hthread)
		r1 = #-1                                        // to set high priority
	}
	{
		memw(H2K_GP+#KG_priomask) = r3                  // H2K_gp->priomask = clrbit(H2K_gp->priomask,hthread)
		p0 = cmp.eq(r15,#255)
		if (p0.new) r1 = #0x000				// enable l2vic interrupt for all priorities
								// including reschedule ...
	}
	imask = r1                                      // set to high priority
	.falign
.H2K_dosched_runlist_push:
	/*--------------------------------------------------*/
	/* H2K_runlist_push(new)                            */
	/*--------------------------------------------------*/
	{
		memb(r17+#CONTEXT_status) = #STATUS_RUNNING     // new->status=H2K_STATUS_RUNNING
		memw(r6+r19<<#2) = r17                          // H2K_gp->runlist[hthread] = newthread
		r6 = add(H2K_GP, #KG_runlist_prios)
	}
	{
		memh(r6+r19<<#1) = r15                          // H2K_gp->runlist_prios[hthread] = new->prio
		r1:0 = combine(r17,r18)                         // func call: r0 = me, r1 = new
		jump H2K_switch                                 // H2K_switch(me, new)
	}

	/*--------------------------------------------------*/
	/* if (new ==NULL)                                  */
	/*--------------------------------------------------*/
	.falign
.H2K_dosched_newthread_null:
	{
		r1 = memw(H2K_GP+#KG_wait_mask)                 // H2K_gp->wait_mask
		r2 = memw(H2K_GP+#KG_priomask)                  // H2K_gp->priomask
	}
	{
		if (r1!=#0) jump:nt .H2K_dosched_newthread_null_cont
		                                                // if(H2K_gp->wait_mask)
		                                                // then done wiht H2K_raise_lowprio()
		r0 = ct0(r2)                                    // ct0(H2K_gp->priomask)
	}
	{
		memw(H2K_GP+#KG_priomask) = #0                  // H2K_gp->priomask = 0
		p0 = r0                                         // change_imask(ct0(mask),-1)
		r1 = #-1                                        // change_imask(ct0(mask),-1)
	}
	{
		setimask(p0,r1)                                 // change_imask(ct0(mask),-1)
	}
	.falign
.H2K_dosched_newthread_null_cont:
	{
		r1:0 = combine(#0,r18)                          // fun call: r0 = me, r1 = NULL
		jump H2K_switch                                 // H2K_switch(me, NULL)
	}
FUNC_END H2K_dosched

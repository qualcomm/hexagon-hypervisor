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
/* Register Usage: R0-R6, R17-R20                                   */
/*------------------------------------------------------------------*/

FUNC_START H2K_dosched  .text.core.dosched  .p2align  3
    /*--------------------------------------------------*/
    /* new = H2K_ready_getbest()                        */
    /*--------------------------------------------------*/
    {
        r19:18 = r1:0                                   // save hthread : me in regs
        r3 = memw(r16+#KG_ready_valids)                 // H2K_gp->ready_valids
        r2 = memw(r16+#KG_ready_validmask)              // H2K_gp->ready_validmask
    }
    {
        r17 = #0                                        // new = NULL
        r0 = and(r3,r2)                                 // H2K_gp->ready_valids& H2K_gp->ready_validmask
    }
    {
        if (r0==#0) jump:t .H2K_dosched_ready_getbestDone                          
                                                        // if (r0 ==0) done with H2K_ready_getbest()
        r0 = ct0(r0)                                    // prio = H2K_ready_ready_best_prio()
    }
    {
        r0 = addasl(r16,r0,#2)                          // for &H2K_gp->ready[prio]
    }
    {
        r17 = memw(r0+#KG_ready)                        // new = H2K_gp->ready[prio]
    }
    {
        r20 = memub(r17+#CONTEXT_prio)                  // new->prio
        r0 = add(r16,#KG_ready)                         // &H2K_gp->ready[0]
    }
    {
        r0 += asl(r20,#2)                               // func call: r0= &H2K_gp->ready[prio]
        r1 = r17                                        // func call: r1= new
        call H2K_ring_remove_real                       // H2K_ring_remove()
    }
    {
        r0 = addasl(r16,r20,#2)                         // for &H2K_gp->ready[prio]
    }
    {
        r0 = memw(r0+#KG_ready)                         // H2K_gp->ready[prio]
    }
    {
        if (r0!=#0) jump:t .H2K_dosched_ready_getbestDone                                                   
                                                        // if (H2K_gp->ready[prio]!=NULL) jump
        r0 = memw(r16+#KG_ready_valids)                 // to update H2K_gp->ready_valids
    }
    {
        r0=clrbit(r0,r20)                               // clrbit(H2K_gp->ready_valids,prio)
    }
    {
        memw(r16+#KG_ready_valids) = r0                 // clrbit(H2K_gp->ready_valids,prio)
    }

.H2K_dosched_ready_getbestDone:
    {
        r1 = memw(r16+#KG_wait_mask)                    // H2K_gp->wait_mask
        p0 = cmp.eq(r17,#0)                             // if(new==NULL) 
        if ( p0.new) jump:nt .H2K_dosched_newthread_null// then jump                        
        if (!p0.new) r4 = memw(r16+#KG_runlist_valids)  // else H2K_gp->runlist_valids
    }
    {
        r2 = memw(r16+#KG_priomask)                     // H2K_gp->priomask
        r5 = memub(r17+#CONTEXT_prio)                   // new->prio
        p0 = cmp.eq(r1,#0)                              // P0 = (H2K_gp->wait_mask==0)
        r0 = cl0(r4)                                    // cl0(H2K_gp->runlist_valids)
    }
    {
        r6 = #STATUS_RUNNING                            //  H2K_STATUS_RUNNING
        p1 = tstbit(r2,r19)                             // P1 = (H2K_gp->priomask&(1<<hthread)!=0)
        if (!p0) jump .H2K_dosched_threadswaitingOrhighprio
                                                        // if !(H2K_gp->wait_mask==0) then jump
        r0 = sub(#31,r0)                                // 8*sizeof(H2K_gp->runlist_valids)-1-cl0(H2K_gp->runlist_valids)
    }
    {
        p0 = cmp.gtu(r5,r0)                             // (new->prio IS_WORSE_THAN H2K_runlist_worst_prio())
        if (!p0.new) jump:nt .H2K_dosched_threadswaitingOrhighprio
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
        p0 = r0                                         // H2K_raise_lowpio() : change_imask(ct0(H2K_gp->priomask),-1)
        r1:0 = combine(#-1,#0)                          // r1 -- H2K_raise_lowpio(): change_imask(,-1)
                                                        // r0 -- to set low priority
        r3 = asl(r3,r19)                                // 1<<hthread
    }
    {
        setimask(p0,r1)                                 // H2K_raise_lowpio() : change_imask(ct0(H2K_gp->priomask),-1)
    }
    { 
        imask = r0                                      // set to low priority 
    }
    {
        memw(r16+#KG_priomask) = r3                     // H2K_gp->priomask |= 1<<thread
        jump .H2K_dosched_runlist_push
    }

    /*--------------------------------------------------*/
    /* else                                             */
    /*--------------------------------------------------*/
.H2K_dosched_threadswaitingOrhighprio:
    {
        if (!p1) jump .H2K_dosched_runlist_push         // if !((H2K_gp->priomask & (1<<hthread))!=0) continue
        r3=clrbit(r2,r19)                               // clrbit(H2K_gp->priomask,hthread)
        NOP; NOP
    }
    {
        memw(r16+#KG_priomask) = r3                     // H2K_gp->priomask=clrbit(H2K_gp->priomask,hthread)
        r0 = #-1                                        // to set high priority
        NOP
    }
    {
        imask = r0                                      // set to high priority 
    }

.H2K_dosched_runlist_push:
    /*--------------------------------------------------*/
    /* H2K_runlist_push(new)                            */
    /*--------------------------------------------------*/
    {
        memb(r17+#CONTEXT_status) = r6                  // new->status = H2K_STATUS_RUNNING
        r4 = setbit(r4,r5)                              // H2K_gp_runlist_valid | (1<<prio)
        r2 = addasl(r16,r5,#2)                          // for &H2K_gp->runlist[prio]
    }
    {
        memw(r16+#KG_runlist_valids) = r4               // H2K_gp_runlist_valid =| (1<<prio)
        r0 = memw(r2+#KG_runlist)                       // H2K_gp->runlist[prio]
    }
    {
        memw(r17+#CONTEXT_next) = r0                    // new->next = H2K_gp->runlist[prio]
    }
    {
        memw(r2+#KG_runlist) = r17                      // H2K_gp->runlist[prio] = new
        r1:0 = combine(r17,r18)                         // func call: r0 = me, r1 = new
        call H2K_switch                                 // H2K_switch(me, new)
    }
    {
        // jumpr r31
    }

.H2K_dosched_newthread_null:
    {
        p0 = cmp.eq(r1,#0)                              // if(H2K_gp->wait_mask) jump
        if (!p0.new) jump:nt .H2K_dosched_newthread_null_cont
        if ( p0.new) r2 = memw(r16+#KG_priomask)        // else H2K_gp->priomask
    }
    {
        memw(r16+#KG_priomask) = r17                    // H2K_gp->priomask = 0
        r0 = ct0(r2)                                    // ct0(mask)
        r1 = #-1                                        // change_imask(ct0(mask),-1)
    }
    {
        p0 = r0                                         // change_imask(ct0(mask),-1)
    }
    {
        setimask(p0,r1)                                 // change_imask(ct0(mask),-1)
    }
.H2K_dosched_newthread_null_cont:
    {
        call H2K_check_sched_mask
    }
    {
        r1:0 = combine(r17,r18)                         // func call: r0 = me, r1 = new
        call H2K_switch                                 // H2K_switch(me, new)
    }
    {
        // jumpr r31
    }
FUNC_END H2K_dosched

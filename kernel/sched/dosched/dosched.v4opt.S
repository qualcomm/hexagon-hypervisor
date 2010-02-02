/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* dosched.v4opt.S
 * This contains fast implementation of dosched.c function
 * for V4
 */

#include <asm_std.h>
#include <asm_offsets.h>
                                                                   
                                                                   
/*------------------------------------------------------------------*/
/* H2K_dosched(H2K_thread_context *me, u32_t hthread)               */
/*------------------------------------------------------------------*/
/* Register Usage: R0-R7, R17-R22                                   */
/*------------------------------------------------------------------*/

FUNC_START H2K_dosched  .text.h2.crit.dosched  .p2align  3
    /*--------------------------------------------------*/
    /* new = H2K_ready_getbest()                        */
    /*--------------------------------------------------*/
    {
        r17 = #0                                        // set new =NULL
        r19:18 = r1:0                                   // save hthread : me in r19:18
        r0 = memw(r16+#KG_ready_valids)                 // H2K_gp->ready_valids
        r1 = memw(r16+#KG_ready_validmask)              // H2K_gp->ready_valid_mask
    }
    {
        r0 = and(r1,r0)                                 // H2K_gp->ready_valids&H2k_gp->ready_validmask
        if (cmp.eq(r0.new,#0)) jump:nt .H2K_dosched_ready_getbestDone              
                                                        // if (r0==0) done with H2K_ready_getbest()
    }
    {
        r2 = ct0(r0)                                    // prio = H2k_ready_best_prio()
        r22 = add(r16,#KG_ready)                        // &H2K_gp->ready[0]
    }
    {
        r17 = memw(r22+r2<<#2)                          // ret = H2K_gp->ready[pio]
    }
    {
        r20 = memub(r17+#CONTEXT_prio)                  // prio = ret->prio
    }
    {
        r0 = addasl(r22,r20,#2)                         // func call: r0 = &H2K_gp->ready[prio]
        r1 = r17                                        // func call: r1 = ret
        call H2K_ring_remove_real                       // H2K_ring_remove()
        r21 = #-1                                       // 0xFFFFFFFF
    }
    {
        r0 = memw(r22+r20<<#2)                          // H2K_gp->ready[pio]
        if (!cmp.eq(r0.new,#0)) jump:t .H2K_dosched_ready_getbestDone              
                                                        // if (H2K_gp->ready[pio]!=NULL) jump
        r2 = clrbit(r21,r20)                            // mask = clrbit(0xFFFFFFFF,prio)
    }
    {
        memw(r16+#KG_ready_valids) &= r2                // H2K_gp->ready_valids = clrbit(H2K_gp->ready_valids,prio)
    }

.H2K_dosched_ready_getbestDone:
    {   
        r1 = memw(r16+#KG_wait_mask)                    // H2K_gp->wait_mask
        r2 = memw(r16+#KG_priomask)                     // H2K_gp->priomask
        if (r17==#0) jump:nt .H2K_dosched_newthread_null// if (new==NULL) then jump             
    }
    {
        p0 = cmp.eq(r1,#0)                              // P0 = (H2K_gp->wait_mask ==0)
        p1 = tstbit(r2,r19)                             // P1 = (H2K_gp->priomask &(1<<hthread) !=0 )
        r3 = lsl(#1,r19)                                // 1<<hthread
        r4 = memw(r16+#KG_runlist_valids)               // H2K_gp->runlist_valids
    }
    {
        r5 = memub(r17+#CONTEXT_prio)                   // new->prio
        r6 = add(r16, #KG_runlist)                      // &H2K_gp->runlist[0]
        if (!p0) jump .H2K_dosched_threadswaitingOrhighprio          
        r0 = cl0(r4)                                    // cl0(H2K_gp->runlist_valids)
    }
    {
        r0 = sub(#31,r0)                                // 8*sizeof(H2K_gp->runlist_valids)-1-cl0(H2K_gp->runlist_valids)
        if (!cmp.gtu(r5,r0.new)) jump:nt .H2K_dosched_threadswaitingOrhighprio          
                                                        // if !( new->prio IS_WORSE_THAN H2K_runlist_worst_prio())
                                                        // then jump to else branch

    /*--------------------------------------------------*/
    /* if((H2K_gp->wait_mask==0) && (new->prio          */
    /*       IS_WORSE_THAN H2K_runlist_worst_prio()))   */
    /*--------------------------------------------------*/
        r1 = ct0(r2)                                    // H2K_raise_lowprio() : ct0(H2K_gp->priomask)
    }
    {
        r7 = memw(r6+r5<<#2)                            // H2K_gp->runlist[new->prio]
        if (p1) jump .H2K_dosched_runlist_push          // if(H2K_gp->priomask &(1<<hthtread) !=0) continue
        p0 = r1                                         // H2K_raise_lowprio(): change_imask(ct0(H2K_gp->priomask),-1)
        r1:0 = combine(#-1,#0)                          // r1 -- H2K_raise_lowprio(): change_imask(,-1)
                                                        // r0 -- to set low priority
    }
    {                                                   // r1 = #-1
        setimask(p0,r1)                                 // H2K_raise_lowprio(): change_imask(ct0(mask),-1)
    }
    {
        imask = r0                                      // set to low priority 
    }
    {
        memw(r16+#KG_priomask) = r3                     // H2k_gp->priomask |= 1<<hthread, (previous = 0)
        jump .H2K_dosched_runlist_push
    }

    /*--------------------------------------------------*/
    /* else                                             */
    /*--------------------------------------------------*/
.H2K_dosched_threadswaitingOrhighprio:  
    {
        r7 = memw(r6+r5<<#2)                            // H2K_gp->runlist[new->prio]
        if (!p1) jump .H2K_dosched_runlist_push         // if(H2K_gp->priomask &(1<<hthtread) ==0) continue
        r3 = and(r2,~r3)                                // clrbit(H2K_gp->priomask,hthread)
        NOP
    }
    {
        memw(r16+#KG_priomask) = r3                     // H2K_gp->priomask = clrbit(H2_gp->priomask,hthread)
        r1 = #-1                                        // to set high priority
        NOP; NOP
    }
    {
        imask = r1                                     // set to high priority 
    }

.H2K_dosched_runlist_push:
    /*--------------------------------------------------*/
    /* H2K_runlist_push(new)                            */
    /*--------------------------------------------------*/
    {   memb(r17+#CONTEXT_status) = #H2K_STATUS_RUNNING // new->status=H2K_STATUS_RUNNING
        memw(r17+#CONTEXT_next) = r7                    // new->next = H2K_gp->runlist[prio]
        r4 = setbit(r4,r5)                              // H2K_gp->runlist_valids | (1<<prio)
    }
    {
        memw(r6+r5<<#2) = r17                           // H2K_gp->runlist[prio] = newthread
        memw(r16+#KG_runlist_valids) = r4               // H2K_gp->runlist_valids |= 1<<prio
        r1:0 = combine(r17,r18)                         // func call: r0 = me, r1 = new
        call H2K_switch                                 // H2K_switch(me, new)
    }
    {
        //jumpr r31
    }

    /*--------------------------------------------------*/
    /* if (new ==NULL)                                  */
    /*--------------------------------------------------*/
.H2K_dosched_newthread_null:
    {
        if (r1!=#0) jump:nt .H2K_dosched_newthread_null_cont             
                                                        // if(H2K_gp->wait_mask)
                                                        // then done wiht H2K_raise_lowprio()
        r0 = ct0(r2)                                    // ct0(H2K_gp->priomask)
    }
    {
        memw(r16+#KG_priomask) = #0                     // H2K_gp->priomask = 0
        p0 = r0                                         // change_imask(ct0(mask),-1)
        r1 = #-1                                        // change_imask(ct0(mask),-1)
    }
    {
        setimask(p0,r1)                                 // change_imask(ct0(mask),-1)
    }
.H2K_dosched_newthread_null_cont:
    {
        call H2K_check_sched_mask
    }
    {
        r1:0 = combine(#0,r18)                          // fun call: r0 = me, r1 = NULL
        call H2K_switch                                 // H2K_switch(me, NULL)
    }
    {
        //jumpr r31
    }
FUNC_END H2K_dosched

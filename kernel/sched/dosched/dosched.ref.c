/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <lowprio.h>
#include <readylist.h>
#include <runlist.h>
#include <check_sanity.h>
#include <switch.h>
#include <globals.h>
#include <dosched.h>

void H2K_dosched(H2K_thread_context *me,u32_t hthread)
{
	H2K_thread_context *new;
	new = H2K_ready_getbest(hthread);
	if (new == NULL) {
		change_imask(hthread,0); //This is not needed actually. 
								// Thing is that if we dont put this here-it is not working.
		/* GO TO SLEEP */
		/* FIXME: temporary ugly hack for broken 8.7+ compiler -- investigate
		 * whether this is still needed and if a proper direct call can be
		 * restored */
		asm volatile ("r0 = %0\n"
									"r1 = #0\n"
									"call H2K_switch\n"
									: : "r"(me));
		//		H2K_switch(me,NULL);
		/* EJP: should never get here! */
	}
	new->hthread = (u8_t)hthread;
	H2K_runlist_push(new);  // This callsite hides inside it a race.
							// If we delete the writes to the "dummy" arrays H2K_runlist_push- futex_pi test hangs.
							// THere is no point in writing to the dummies, just for this callsit, 
							// All other callsites across the code are fine without H2K_runlist_push writing 
							// to the dummies, but this preserved for futex_pi test to pass.
	//if (new->vmstatus & H2K_VMSTATUS_VMWORK) H2K_vm_do_work(new);
	H2K_switch(me,new);
	/* EJP: should never get here! */
}


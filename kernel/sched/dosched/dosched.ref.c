/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <check_sanity.h>
#include <switch.h>
#include <globals.h>
#include <dosched.h>

void H2K_dosched(H2K_thread_context *me,u32_t hthread)
{
	H2K_thread_context *new;
	new = H2K_ready_getbest(hthread);
	if (new == NULL) {
		/* GO TO SLEEP */
		/* FIXME: temporary ugly hack for broken 8.7+ compiler -- investigate
		 * whether this is still needed and if a proper direct call can be
		 * restored */
		asm volatile ("r0 = %0\n"
									"r1 = #0\n"
									"call H2K_switch\n"
									: : "r"(me));
		//	H2K_switch(me,NULL);
		/* EJP: should never get here! */
	}
	new->hthread = (u8_t)hthread;
	//if (new->vmstatus & H2K_VMSTATUS_VMWORK) H2K_vm_do_work(new);
	new->status = H2K_STATUS_RUNNING;
	H2K_switch(me,new);
	/* EJP: should never get here! */
}


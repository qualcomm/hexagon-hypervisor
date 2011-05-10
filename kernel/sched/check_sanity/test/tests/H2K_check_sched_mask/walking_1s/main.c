/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdlib.h>
#include <c_std.h>
#include <check_sanity.h>
#include <hw.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>
#include <globals.h>

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

/*  This test covers check_sched_mask, which is inlined into H2K_check_sanity as well as 
    called independently elsewhere (and has an optimized version)  */

/* Strategy:
 * Make the machine uninterruptible by clearing the GIE bit. 
 * Then, try different inputs to H2K_check_sched_mask and see whether or not
 * reschedule interrupt was raised.
 */

void FAIL(const char *str) 
{
	h2_printf(str);
	exit(1);
}

void print_test_defines(void) 
{
	info("MAX_PRIOS = %d\n",MAX_PRIOS);
	info("MAX_HTHREADS = %d\n",MAX_HTHREADS);

}

/*  Checks if the reschedule was requested (check ipend); also clears it */
int resched_requested()
{
	u32_t ipend;
	ipend = H2K_get_ipend();

	if (ipend & RESCHED_INT_INTMASK) return 1;
	return 0;
}

/*  Checks if lowprio_notify was requested (get imasks?)  */

int lowprio_notify_requested(u32_t correctmask)
{
	/*  Need to check if H2K_gp->priomask actually has the correct bit(s) set now  */
	if (correctmask == H2K_gp->priomask) return 1;
	return 0;
}

static inline int setbit(int nr) {
	int retval;
	asm volatile("R1 = #0; %0 = setbit(R1,%1);\n" 
			: "=r" (retval)
			: "r" (nr)
			: "r1" );
	return retval;
}

int main() 
{
	u32_t prio_hthread;	/*  bitmap indexed by hardware thread -- shows lowest priority hthread  */
	u32_t runlist_prio;	/*  bitmap indexed by priority -- currently running priorities  */
	int i,j,l;		/*  ubiquitous loop counter variables  */

	u32_t check_mask;	/*  "correct" value for lowprio_notify to check  */

	u32_t global_valid_prio;  /*  bitmap indexed by priority -- EJP calls this "ready_validmask"; determines
				      which priorities are allowed to be either running or ready; 0 indicates neither.  */

	u32_t resched_count=0;  /*  need to distinguish between sightings of the two different reasons for rescheds  */
	u32_t lowprio_count=0;

	/*  Call h2_init() to initialize everything  */

	h2_init(0x0);

	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	info("%s starting\n",__FUNCTION__);

	for (i=0; i<1000; i++) {
		asm volatile("nop;");
	}  /*  h2_init may need some settling time for other threads.  */

	/*  disable the global IE bit and clear ipend  */
	H2K_clear_gie();
	H2K_clear_ipend(0xffffffff);

	print_test_defines();

	/*  Walking 1's test  
	 * 
	 *  This testcase should never actually schedule out.
	 *
	 *  Walk 1 across H2K_priomask - MAX_HTHREADS positions
	 *  Walk 1 across H2K_runlist_valids -- MAX_PRIOS positions
	 *  Walk 1 across H2K_ready_valids -- MAX_PRIOS positions
	 */

	/*  Fudge the H2K_runlist[prio]->hthread to a bogus value so we can properly 
	 *  detect H2K_lowprio_notify() actually fired.  Might cause simulator problems...
	 */

	for (i=0; i<MAX_PRIOS; i++) {
		/* 
		 * aka go from 7 to 31 and wrap around to 7 again; not exactly full coverage
		 * since we're avoiding the first hardware threads for checking, but should
		 * be mostly correct.
		 */
		H2K_gp->runlist[i]->hthread=(MAX_HTHREADS) + (i % (MAX_PRIO-MAX_HTHREADS));  
	}

	//  If this were Python, I'd do this:
	//  global_valid_prio	= [0,1,3,7,15,31] and empty set makes 7.
	//  prio_hthread	= [0,1,2,4,8,16,32]
	//  runlist_prio	= [1,3,7,15,31] # (apparently 0 is not a valid runlist state?)

	global_valid_prio = 0;
	for (i=0; i<7; i++) {
		if (i != 0) {
			global_valid_prio = (1<<(i-1))-1;
			global_valid_prio = setbit(global_valid_prio);
		}
		//info("global_valid_prio = 0x%08x\n",global_valid_prio);
		prio_hthread = 0;
		for (j=0; j<(MAX_HTHREADS+1); j++) {
			if (j != 0) {
				prio_hthread = prio_hthread ? 1 << (j-1) : 1;
			}
			for (l=0; l<6; l++) {
				runlist_prio = (1<<l)-1;
				runlist_prio = setbit(runlist_prio);
				//info("runlist_prio = 0x%08x\n",runlist_prio);

				H2K_gp->priomask = prio_hthread;
				H2K_gp->runlist_valids = runlist_prio;
				H2K_gp->ready_validmask = ~global_valid_prio;
			
				//debug("prio_hthread = 0x%08x\n",prio_hthread);
				//debug("runlist_prio = 0x%08x\n",runlist_prio);
				//debug("global_valid_prio = 0x%08x\n",~global_valid_prio);

				//debug("runlist_worst_prio %d\n",H2K_runlist_worst_prio());

				//  calculate prior to butchering
				check_mask = prio_hthread | (1 << H2K_gp->runlist[H2K_runlist_worst_prio()]->hthread);

				//debug("check_mask = 0x%08x\n",check_mask);
				//debug("prio_hthread = 0x%08x\n",prio_hthread);
				//debug("runlist_prio = 0x%08x\n",runlist_prio);
				//debug("global_valid_prio = 0x%08x\n",~global_valid_prio);
				//debug("H2K_gp->priomask = 0x%08x\n",H2K_gp->priomask);

				H2K_check_sched_mask();

				/*  check the results  */
				/*  Was a resched necessary?  */

				/*  
				 * if runlist & ~H2K_ready_validmask, then somebody needs
				 * a boot to the head. 
				 */

				if ((runlist_prio & global_valid_prio) != 0) {
					if (!resched_requested()) {
						error("Expected reschedule due to ready_validmask\n");
					}
					resched_count++;
					goto sched_fired_ok;
				}

				/*  Should NOT have seen a scheduler fire at this point  */
				if (resched_requested()) {
					debug("prio_hthread = 0x%08x\n",prio_hthread);
					debug("runlist_prio = 0x%08x\n",runlist_prio);
					debug("global_valid_prio = 0x%08x\n",~global_valid_prio);
					error("Inappropriate reschedule requested()\n");
				}
sched_fired_ok:
				/*  Calculate the size of the boot for this check.  */
					
				if ((runlist_prio & global_valid_prio) != 0) {
					if (!lowprio_notify_requested(check_mask)) {
						debug("check_mask = 0x%08x\n",check_mask);
						debug("prio_hthread = 0x%08x\n",prio_hthread);
						debug("runlist_prio = 0x%08x\n",runlist_prio);
						debug("global_valid_prio = 0x%08x\n",~global_valid_prio);
						debug("H2K_gp->priomask = 0x%08x\n",H2K_gp->priomask);
						error("Expected lowprio_notify due to ready_validmask\n");
					}
					lowprio_count++;
					goto lowprio_ok;
				}

				if (lowprio_notify_requested(check_mask)) {
					error("Inappropriate lowprio notify detected\n");
				}

lowprio_ok:
				/*  cleanup  */
				H2K_clear_ipend(RESCHED_INT_INTMASK);

			}
		}
	}

	/* Todo:  Compare against a hand calculated number of notifications and resched ints fired.  */

	info("Totals:\n");
	info("resched_count = %d\n",resched_count);
	info("lowprio_count = %d\n",lowprio_count);

	/*  
	 * Should only reach here if everything passed.  Assertions and errors should kill the test before 
	 * we ever get here.  Multithreaded tests should reach some sort of synchronization to prevent the
	 * main thread from exiting and reporting passd before all tests have completed.  
	 */

	info("TEST PASSED\n");
	exit(0);
}


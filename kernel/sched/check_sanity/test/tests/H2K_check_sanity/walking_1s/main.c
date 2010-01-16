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

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

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

int lowprio_notify_requested(u32_t oldmask)
{

	/*  Can also check if the old priority mask matches the new one  */
	if (oldmask != H2K_priomask) return 1;
	return 0;
}

int main() 
{
	u32_t prio_hthread;  /*  bitmap indexed by hardware thread -- shows lowest priority hthread  */
	u32_t wait_hthread;  /*  bitmap indexed by hardware thread -- shows which thread is in wait mode  */
	u32_t runlist_prio;  /*  bitmap indexed by priority -- currently running priorities  */
	u32_t ready_prio;    /*  bitmap indexed by priority -- which priorities have tasks ready to run  */

	u32_t global_valid_prio;  /*  bitmap indexed by priority -- EJP calls this "ready_validmask"; determines
				      which priorities are allowed to be either running or ready; 0 indicates neither.  */

	u32_t some_random_number;  /*  checked by assertions  */
	u32_t retval;  /*  checked by assertions  */

	u32_t resched_count=0;  /*  need to distinguish between sightings of the two different reasons for rescheds  */
	u32_t lowprio_count=0;
	int i;

	/*  Call h2_init() to initialize everything  */

	h2_init(0x0);

	info("%s starting\n",__FUNCTION__);

	for (i=0; i<1000; i++) {
		asm volatile("nop;");
	}  /*  h2_init may need some settling time for other threads.  */

	/*  disable the global IE bit and clear ipend  */
	H2K_clear_gie();
	H2K_clear_ipend(0xffffffff);

	/*  Walking 1's test  
	 * 
	 *  This testcase should never actually schedule out.
	 *
	 *  Walk 1 across H2K_priomask - MAX_HTHREADS positions
         *  Walk 1 across H2K_wait_mask - MAX_HTHREADS positions
	 *  Walk 1 across H2K_runlist_valids -- MAX_PRIOS positions
	 *  Walk 1 across H2K_ready_valids -- MAX_PRIOS positions
	 */

	/*  
	 * For MAX_HTHREADS == 6 and MAX_PRIOS == 32, this means:
 	 *
	 * 7 * 7 * 33 * 33 = 53361 combinations right here
	 *
	 * (I'm including zero's in there) 
         *
         * for ready_validmask, I think I'm going to walk a zero.
         */

	for (prio_hthread=0; prio_hthread < (1<<(MAX_HTHREADS-1)); prio_hthread = prio_hthread ? prio_hthread << 1 : 1) {
		for (wait_hthread=0; wait_hthread < (1<<(MAX_HTHREADS-1)); wait_hthread = wait_hthread ? wait_hthread<<1 : 1) {
			for (runlist_prio=1; runlist_prio < (1<<(MAX_PRIOS-1)); runlist_prio = runlist_prio ? runlist_prio<<1 : 1) {
				for (ready_prio=0; ready_prio < (1<<(MAX_PRIOS-1)); ready_prio = ready_prio ? ready_prio <<=1 : 1) {
					for (global_valid_prio=0; global_valid_prio < (1<<(MAX_PRIOS-1)); global_valid_prio = global_valid_prio ? global_valid_prio <<=1 : 1) {
						H2K_priomask = prio_hthread;
						H2K_wait_mask = wait_hthread;
						H2K_runlist_valids = runlist_prio;
						H2K_ready_valids = ready_prio;
						//H2K_ready_validmask = ~global_valid_prio;
					
						//debug("prio_hthread = 0x%08x\n",prio_hthread);
						//debug("wait_hthread = 0x%08x\n",wait_hthread);
						//debug("runlist_prio = 0x%08x\n",runlist_prio);
						//debug("ready_prio = 0x%08x\n",ready_prio);

						//debug("runlist_worst_prio %d\n",H2K_runlist_worst_prio());
						//debug("ready_best_prio %d\n",H2K_ready_best_prio());

						some_random_number = rand();
						/*  call the function  */
						BKL_LOCK();  /*  Big kernel lock required  */
						retval = call(H2K_check_sanity_unlock,some_random_number);

						/*  check the results  */
						/*  Was a resched necessary?  */
	
						/* 
						 * If "IS_WORSE_THAN" changes then 
						 * this has to be changed as well.
						 */
	
						if ((ready_prio&global_valid_prio) && (runlist_prio > (ready_prio&global_valid_prio))) {
							if (!resched_requested()) {
								error("Expected reschedule due to better ready tasks\n");
							}
							resched_count++;
							goto sched_fired_ok;
						}  /*  best ready is better than worst running  */
	
						if ((wait_hthread != 0) && ((ready_prio*global_valid_prio)!= 0)) {
							if (!resched_requested()) {
								debug("wait_hthread == 0x%x, ready_prio == 0x%x\n",wait_hthread, ready_prio);
								error("Expected reschedule due to idle hardware thread with ready tasks\n");
							}
							resched_count++;
							goto sched_fired_ok;
						}  /*  tasks ready to go while thread is in wait  */
	
						/*  
						 * if runlist & ~global_valid_prio, then somebody needs
						 * a boot to the head.
						 */

						if (runlist_prio && ~global_valid_prio) {
							if (!resched_requested()) {
								error("Expected reschedule due to ready_validmask\n");
							}
							resched_count++;
							goto sched_fired_ok;
						}

						/*  Should NOT have seen a scheduler fire at this point  */
						if (resched_requested()) {
							error("Inappropriate reschedule requested()\n");
						}
sched_fired_ok:
						/*  Was a lowprio_notify necessary?  */
						
						if (prio_hthread == 0) {
							if (!lowprio_notify_requested(prio_hthread)) {
								error("Lowprio notify not detected\n");
							}
							lowprio_count++;
							goto lowprio_ok;
						} 

						if (runlist_prio && ~global_valid_prio) {
							if (!lowprio_notify_requested(prio_hthread)) {
								error("Expected lowprio_notify due to ready_validmask\n");
							}
							lowprio_count++;
							goto sched_fired_ok;
						}

						if (lowprio_notify_requested(prio_hthread)) {
							error("Inappropriate lowprio notify detected\n");
						}

lowprio_ok:
						/*  cleanup  */
						H2K_clear_ipend(RESCHED_INT_INTMASK);
	
					}
				}
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


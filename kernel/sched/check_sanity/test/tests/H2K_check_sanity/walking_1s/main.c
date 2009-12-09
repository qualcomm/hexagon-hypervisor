/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <check_sanity.h>
#include <hw.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>

#define info(...) printf(__VA_ARGS__);
#define warn(...) printf(__VA_ARGS__);
#define debug(...) printf(__VA_ARGS__);

/*  Todo:  add a "fail" function mandatory for all tests; error can call it.  */
#define error(...) printf(__VA_ARGS__);

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
	u64_t prio_hthread;  /*  bitmap indexed by hardware thread -- shows lowest priority hthread  */
	u64_t wait_hthread;  /*  bitmap indexed by hardware thread -- shows which thread is in wait mode  */
	u32_t runlist_prio;  /*  bitmap indexed by priority -- currently running priorities  */
	u32_t ready_prio;    /*  bitmap indexed by priority -- which priorities have tasks ready to run  */

	u32_t some_random_number;  /*  checked by assertions  */

	/*  Call h2_init() to initialize everything  */

	h2_init();

	info("%s starting\n",__FUNCTION__);

	/*  disable the global IE bit and clear ipend*/
	H2K_clear_ie();
	H2K_clear_ipend(0xffffffff);

	/*  Walking test  
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
	 * 6 * 6 * 32 * 32 = 36864 combinations right here
	 *
         */

	for (prio_hthread=1; prio_hthread < (1<<MAX_HTHREADS-1); prio_hthread <<= 1) {
		for (wait_hthread=1; wait_hthread < (1<<MAX_HTHREADS-1); wait_hthread<<=1) {
			for (runlist_prio=1; runlist_prio < (1<<MAX_PRIOS-1); runlist_prio<<=1) {
				for (ready_prio=1; ready_prio < (1<<MAX_PRIOS-1); ready_prio <<=1) {
					H2K_priomask = prio_hthread;
					H2K_wait_mask = wait_hthread;
					H2K_runlist_valids = runlist_prio;
					H2K_ready_valids = ready_prio;

					/*  call the function  */
					H2K_check_sanity(some_random_number);

					/*  check the results  */

					/*  cleanup  */
					H2K_clear_ipend(0xffffffff);

				}
			}
		}
	}

	/*  
	 * Should only reach here if everything passed.  Assertions and errors should kill the test before 
	 * we ever get here.  Multithreaded tests should reach some sort of synchronization to prevent the
	 * main thread from exiting and reporting passd before all tests have completed.  
	 */

	printf("TEST PASSED\n");
}


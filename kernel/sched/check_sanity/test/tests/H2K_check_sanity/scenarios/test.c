/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdlib.h>
#include <stdio.h>
#include <context.h>
#include <readylist.h>
#include <runlist.h>
#include <lowprio.h>
#include <check_sanity.h>
#include <hw.h>
#include <globals.h>

#define NUM_THREADS_IN_SCENARIOS 12  // must match the Python script

enum status {
	BLOCKED,
	READY,
	RUNNING
};

struct thread_state {
	enum status status;
	int hthread;
	int prio;
};

struct scenario {
	struct thread_state thread[NUM_THREADS_IN_SCENARIOS];
	int should_resched;
	int hthreads;
};

static char *status_string[] = {
	"BLOCKED",
	"READY",
	"RUNNING"
};

static int test_status_to_H2_status[] = {
	H2K_STATUS_BLOCKED,
	H2K_STATUS_READY,
	H2K_STATUS_RUNNING
};

static H2K_thread_context threads[NUM_THREADS_IN_SCENARIOS];

/* Get the scenarios from scenarios.h. */
static struct scenario scenarios[] = {
#include "scenarios.h"
};

/* Prints a scenario. */
static void print_scenario(struct scenario *scenario)
{
	int num_threads = sizeof(threads) / sizeof(threads[0]);
	int status;
	int hthread;
	int prio;
	int i;
	puts("Thread  Status   hthread  prio");
	for (i = 0; i < num_threads; i++) {
		status = scenario->thread[i].status;
		hthread = scenario->thread[i].hthread;
		prio = scenario->thread[i].prio;
		printf("t%-5d  %s  %-7d  %d\n", i, status_string[status], hthread, prio);
	}
	printf("should_resched = %d\n", scenario->should_resched);
}

/* Indicates a failure.  Prints the scenario that failed. */
static void FAIL(const char *str, struct scenario *scenario)
{
	puts("FAIL");
	puts(str);
	print_scenario(scenario);
	exit(1);
}

/*  Checks if the resched interrupt was raised. */
static int resched_requested(void)
{
	return (H2K_get_ipend() & RESCHED_INT_INTMASK) != 0;
}

/* Sets up the state described by the scenario. */
static void setup(struct scenario *scenario)
{
	int num_threads = sizeof(threads) / sizeof(threads[0]);
	int status;
	int hthread;
	int prio;
	int i;
	int worst_running_prio = 0;  /* 0 == best; track max over RUNNING threads */

	H2K_clear_ipend(0xffffffff);
	H2K_readylist_init();
	H2K_set_schedcfg(SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT));
	for (i = 0; i < num_threads; i++) {
		status = scenario->thread[i].status;
		hthread = scenario->thread[i].hthread;
		prio = scenario->thread[i].prio;
		threads[i].status = test_status_to_H2_status[status];
		threads[i].hthread = hthread;
		threads[i].prio = prio;
		if (status == RUNNING) {
			H2K_runlist_push(&threads[i]);
			if (prio > worst_running_prio)
				worst_running_prio = prio;
		} else if (status == READY) {
			H2K_ready_append(&threads[i]);
		}
	}
	/* The BESTWAIT comparator reads each hw thread's REAL STID.PRIO (bits
	 * [23:16]), not the software runlist_prios[] array that H2K_runlist_push
	 * fills.  This test runs on a single hw thread, so drive that thread's real
	 * STID.PRIO to the scenario's worst running priority -- the only running
	 * priority that affects the decision (hardware fires iff some thread's
	 * STID.PRIO is worse than BESTWAIT, i.e. worst_running > best_ready).
	 *
	 * The comparator is level-sensitive, so disarm BESTWAIT (0x1FF) BEFORE
	 * writing STID, otherwise a stale BESTWAIT left by the previous scenario
	 * would fire spuriously the instant we install a worse STID.PRIO.  Clear
	 * IPEND last so setup() leaves a clean slate; only H2K_check_sanity (under
	 * test) should arm BESTWAIT and raise the interrupt. */
	H2K_set_bestwait(BESTWAIT_MASK);
	asm volatile ("stid = %0;" : : "r"((u32_t)worst_running_prio << 16));
	H2K_clear_ipend(0xffffffff);
}

/* Checks to see that H2K_check_sanity did its job, given the scenario. */
static void check(struct scenario *scenario)
{
	if (scenario->should_resched && !resched_requested()) {
		FAIL("failed to raise resched interrupt", scenario);
	}
	if (!scenario->should_resched && resched_requested()) {
		FAIL("raised resched interrupt unnecessarily", scenario);
	}
}

static inline u64_t do_H2K_check_sanity(u64_t input)
{
	__asm__ __volatile__ (GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	return H2K_check_sanity(input);
} 

static inline u64_t do_H2K_check_sanity_unlock(u64_t input)
{
	__asm__ __volatile__ (GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	return H2K_check_sanity_unlock(input);
} 

/* Runs through the scenarios, setting them up and knocking them down. */
int main()
{
	int num_scenarios = sizeof(scenarios) / sizeof(scenarios[0]);
	int i;

	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	H2K_gp->hthreads = get_hthreads();

	H2K_clear_gie();
	for (i = 0; i < num_scenarios && scenarios[i].hthreads <= H2K_gp->hthreads; i++) {
		setup(&scenarios[i]);
		BKL_LOCK();
		call(do_H2K_check_sanity, rand());
		BKL_UNLOCK();
		check(&scenarios[i]);

		setup(&scenarios[i]);
		BKL_LOCK();
		call(do_H2K_check_sanity_unlock, rand());
		check(&scenarios[i]);
	}
	puts("TEST PASSED\n");
	return 0;
}

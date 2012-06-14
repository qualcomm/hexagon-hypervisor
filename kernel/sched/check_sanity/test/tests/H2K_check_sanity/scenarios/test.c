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
	int priomask;
	int wait_mask;
	int expected_priomask;
	int should_resched;
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
	printf("priomask = 0x%x\n", scenario->priomask);
	printf("wait_mask = 0x%x\n", scenario->wait_mask);
	printf("expected_priomask = 0x%x\n", scenario->expected_priomask);
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

	H2K_clear_ipend(0xffffffff);
	H2K_readylist_init();
	H2K_runlist_init();
	H2K_lowprio_init();
	for (i = 0; i < num_threads; i++) {
		status = scenario->thread[i].status;
		hthread = scenario->thread[i].hthread;
		prio = scenario->thread[i].prio;
		threads[i].status = test_status_to_H2_status[status];
		threads[i].hthread = hthread;
		threads[i].prio = prio;
		if (status == RUNNING) {
			H2K_runlist_push(&threads[i]);
		} else if (status == READY) {
			H2K_ready_append(&threads[i]);
		}
	}
	H2K_kg.priomask = scenario->priomask;
	H2K_kg.wait_mask = scenario->wait_mask;
}

/* Checks to see that H2K_check_sanity did its job, given the scenario. */
static void check(struct scenario *scenario)
{
	int i;
	if (scenario->priomask == 0) {
		if ((H2K_kg.priomask & scenario->expected_priomask) == 0) {
			FAIL("priomask not set for worst priority thread", scenario);
		}
		for (i = 0; i < MAX_HTHREADS; i++) {
			if (H2K_kg.priomask & 1 << i && get_imask(i) != 0) {
				FAIL("failed to clear imask", scenario);
			}
		}
	} else {
		if (H2K_kg.priomask != scenario->priomask) {
			FAIL("mucked with non-zero priomask", scenario);
		}
	}
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
	H2K_clear_gie();
	for (i = 0; i < num_scenarios; i++) {
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

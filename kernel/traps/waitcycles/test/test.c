/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <h2.h>
#include <hw.h>
#include <hexagon_protos.h>
#include <max.h>
#include <stdio.h>

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define fail(...) { h2_printf("FAIL:  "); h2_printf(__VA_ARGS__); exit(1);}

static void wait(unsigned int time)
{
	int i;
	for (i = 0; i < time; i++) {
		asm volatile("nop");
	}
}

static u64_t get_pcycles(void)
{
	u64_t pcycles;
	asm volatile("r4 = pcyclehi\n\t"
	             "r2 = pcyclelo\n\t"
	             "r3 = pcyclehi\n\t"
	             "p0 = cmp.eq(r3,r4)\n\t"
	             "if (!p0) r2 = #0\n\t"
	             "%0 = r3:2"
	             : "=r" (pcycles) : : "r2", "r3", "r4", "p0" );
	return pcycles;
}

static u64_t get_waitcycles(unsigned int htid)
{
	u64_t waitcycles;
	asm volatile("r0 = %1\n\t"
	             "trap0(#27)\n\t"
	             "%0 = r1:0"
	             : "=r" (waitcycles) : "r" (htid) : "r0", "r1", "r2", "r3",
	                                                "r4", "r5", "r6", "r7",
	                                                "r8", "r9", "r10", "r11",
	                                                "r12", "r13", "r14", "r15",
	                                                "r28", "r31",
	                                                "p0", "p1", "p2", "p3",
	                                                "lc0", "sa0", "lc1", "sa1",
	                                                "m0", "m1", "usr", "ugp" );
	return waitcycles;
}

int main()
{
	u64_t start_time;
	u64_t total_time;
	u64_t waitcycles[MAX_HTHREADS];
	int i;
	int hthreads = get_hthreads();

	h2_init(NULL);

	h2_hwconfig_hwthreads_mask(-1);  // start all hw threads

	start_time = get_pcycles();
	wait(420000);
	total_time = get_pcycles() - start_time;

	for (i = 0; i < hthreads; i++) {
		waitcycles[i] = get_waitcycles(i);
	}

	info("time spinning:  %llu\n", total_time);
	for (i = 0; i < hthreads; i++) {
		info("time hardware thread %d spent waiting:  %llu\n", i, waitcycles[i]);
		if (i && (total_time * 1.1 < waitcycles[i] || waitcycles[i] * 1.1 < total_time)) {
			fail("The wait cycles for hardware thread %llu are more than 10%% off of the time thread 0 spent spinning.\n", i);
		}
	}

	puts("TEST PASSED");
	return(0);
}

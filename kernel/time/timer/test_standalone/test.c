/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <globals.h>
#include <hw.h>
#include <vm.h>
#include <timeinfo.h>
#include <timer.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

H2K_kg_t H2K_kg; /* Prevents bringing in lots of H2 */

#if ARCHV <= 4
u32_t mydev[4];
#define MATCH 0
#define COUNT 1
#define ENABLE 2
#else
u32_t mydev[0x1000];
u32_t intdev[0x1000];
#define COUNT_LO (0x1000/4)
#define COUNT_HI (0x1004/4)
#define MATCH_LO (0x1020/4)
#define MATCH_HI (0x1024/4)
#define ENABLE	 (0x102C/4)
#endif

u32_t TH_expected_cpuint_post = 0;

s32_t H2K_vm_cpuint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno)
{
	if (TH_expected_cpuint_post == 0) FAIL("Unexpected post");
	if (intno != H2K_TIME_GUESTINT) FAIL("cpuint wrong int posted");
	if (dest != &a) FAIL("cpuint wrong dest");
	if (vmblock != a.vmblock) FAIL("cpuint wrong vmblock");
	if (a.timeout != 0) FAIL("timeout not reset");
	TH_expected_cpuint_post = 0;
	return 0;
}

void TH_reset()
{
	a.timeout = a.rightleft = 0ULL;
	a.vmblock = NULL;
	H2K_gp->time.last_ticks = 0ULL;
	H2K_gp->time.next_ticks = ~0ULL;
	H2K_gp->time.timeouts = NULL;
}

#if ARCHV <= 4
int main()
{
	u64_t ret;
	u64_t resolution;
	double error;
	H2K_thread_context *me = &a;

        __asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));

	/* Test bad trap */
	if ((ret = H2K_timer_trap(100,0,me)) != ~0ULL) FAIL("bad return on bad trap");

	H2K_gp->time.devptr = mydev;
	H2K_gp->time.last_ticks = 0;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_FREQ,0,me)) < 100) FAIL("bad freq");
	error = ((double)ret / 1e9);
	if (error < 1.0) error = 2.0 - error;
	if (error > 1.1) FAIL("Freq not near nanoseconds");

	if ((resolution = H2K_timer_trap(H2K_TIMER_TRAP_GET_RESOLUTION,0,me)) <= 2) {
		FAIL("resolution");
	}

	mydev[COUNT] = 1;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIME,0,me)) != resolution) {
		printf("ret: 0x%016llx resolution=0x%016llx\n",ret,resolution);
		FAIL("time/1");
	}
	if (H2K_gp->time.last_ticks != 1) FAIL("time/last ticks");

	mydev[COUNT] = 2;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIME,0,me)) != 2*resolution) FAIL("time/2");
	if (H2K_gp->time.last_ticks != 2) FAIL("time/last ticks");

	mydev[COUNT] = 0;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIME,0,me)) != (resolution << 32)) {
		FAIL("time/wrap");
	}
	if (H2K_gp->time.last_ticks != 0x100000000ULL) FAIL("time/last ticks");

	a.timeout = 0x12345;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIMEOUT,0,me)) != (0x12345 * resolution)) {
		FAIL("get timeout");
	}

	/* Basic functionality: timer in the future */
	TH_reset();
	a.timeout = 0;
	H2K_gp->time.last_ticks = 2;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x12340ULL * resolution,me)) 
		!= (0x12340 * resolution)) FAIL("set timeout return");
	if (a.timeout != 0x12340) FAIL("bad ticks calculation");
	if (H2K_gp->time.timeouts == NULL) FAIL("not put to tree");

	TH_reset();
	mydev[COUNT] = 2;
	mydev[MATCH] = 0;
	H2K_timer_int(0,NULL,0);

	if (mydev[MATCH] <= 0x40000000) {
		printf("new match: 0x%08x\n",mydev[MATCH]);
		FAIL("Didn't set timer for future");
	}
	if (H2K_gp->time.last_ticks == 0) FAIL("Didn't update last_ticks");

	/* Test some cases that should invalidate active timer: BIGBANG and
	 * FOREVER
	 */
	TH_reset();
	a.timeout = 0;
	H2K_gp->time.last_ticks = 2;

	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x12340ULL * resolution,me)) 
		!= (0x12340 * resolution)) FAIL("set timeout return");
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x0ULL,me)) != 0) FAIL("set bigbang return");
	if (H2K_gp->time.timeouts != NULL) FAIL("didn't clear out of timeout tree");
	if (a.timeout != 0) FAIL("didn't clear timeout");

	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x12340ULL * resolution,me)) 
		!= (0x12340 * resolution)) FAIL("set timeout return");
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,~0x0ULL,me)) != 0) FAIL("set forever return");
	if (H2K_gp->time.timeouts != NULL) FAIL("didn't clear out of timeout tree");
	if (a.timeout != 0) FAIL("didn't clear timeout");

	/* Test timer interrupt */

	TH_reset();
	a.timeout = 0x12345;
	mydev[COUNT] = 2;
	mydev[MATCH] = 0;
	H2K_gp->time.timeouts = &a.tree;
	H2K_timer_int(0,NULL,0);

	if (mydev[MATCH] != 0x12345) {
		printf("new match: 0x%08x\n",mydev[MATCH]);
		FAIL("Didn't set timer for next interrupt");
	}
	if (H2K_gp->time.last_ticks == 0) FAIL("didn't update last_ticks");
	if (H2K_gp->time.timeouts != &a.tree) FAIL("erased unexpired timeout");

	mydev[COUNT] = 0x12345;
	TH_expected_cpuint_post = 1;
	H2K_timer_int(0,NULL,0);
	if (TH_expected_cpuint_post != 0) FAIL("Didn't call cpuint");
	if (H2K_gp->time.timeouts != NULL) FAIL("Didn't empty timeout tree");
	if (H2K_gp->time.last_ticks != 0x12345) FAIL("Didn't update last_ticks");
	if (mydev[MATCH] == 0x12345) FAIL("didn't set timer for next interrupt");

	H2K_gp->time.timeouts = &a.tree;
	mydev[MATCH] = 0x12345;
	mydev[COUNT] = 0x0;
	mydev[ENABLE] = 0x0;
	mydev[3] = 0x0;
	H2K_timer_init();
	if (mydev[MATCH] == 0x12345) FAIL("init/match");
	if (mydev[COUNT] != 0x0) FAIL("init/count");
	if (mydev[ENABLE] != 0x1) FAIL("init/enable");
	if (H2K_gp->time.timeouts != NULL) FAIL("init/timeouts");
	if (H2K_gp->time.last_ticks != 0) FAIL("ticks");

	H2K_gp->time.devptr = NULL;
	H2K_timer_init();
	if (H2K_gp->time.devptr != ((void *)TIMER_BASE_VA)) FAIL("reset devptr");
	H2K_gp->time.devptr = mydev;

	/* Test timer interrupt -- wrapping ticks when 32-bit timer wraps around */
	TH_reset();
	mydev[COUNT] = 0x40000000;
	H2K_timer_int(0,NULL,0);
	mydev[COUNT] = 0x30000000;
	H2K_timer_int(0,NULL,0);
	if (H2K_gp->time.last_ticks != 0x0000000130000000ULL) FAIL("wrap");

	puts("TEST PASSED\n");
	return 0;
}
#else
int main()
{
	u64_t ret;
	u64_t resolution;
	double error;
	H2K_thread_context *me = &a;

        __asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));

	/* Test bad trap */
	if ((ret = H2K_timer_trap(100,0,me)) != ~0ULL) FAIL("bad return on bad trap");

	H2K_gp->time.devptr = mydev;
	H2K_gp->time.last_ticks = 0;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_FREQ,0,me)) < 100) FAIL("bad freq");
	error = ((double)ret / 1e9);
	if (error < 1.0) error = 2.0 - error;
	if (error > 1.1) FAIL("Freq not near nanoseconds");

	if ((resolution = H2K_timer_trap(H2K_TIMER_TRAP_GET_RESOLUTION,0,me)) <= 2) {
		FAIL("resolution");
	}

	mydev[COUNT_LO] = 1;
	mydev[COUNT_HI] = 0;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIME,0,me)) != resolution) {
		printf("ret: 0x%016llx resolution=0x%016llx\n",ret,resolution);
		FAIL("time/1");
	}
	if (H2K_gp->time.last_ticks != 1) FAIL("time/last ticks");

	mydev[COUNT_LO] = 2;
	mydev[COUNT_HI] = 0;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIME,0,me)) != 2*resolution) FAIL("time/2");
	if (H2K_gp->time.last_ticks != 2) FAIL("time/last ticks");

	mydev[COUNT_LO] = 0;
	mydev[COUNT_HI] = 1;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIME,0,me)) != (resolution << 32)) {
		FAIL("time/wrap");
	}
	if (H2K_gp->time.last_ticks != 0x100000000ULL) FAIL("time/last ticks");

	a.timeout = 0x12345;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_GET_TIMEOUT,0,me)) != (0x12345 * resolution)) {
		FAIL("get timeout");
	}

	/* Basic functionality: timer in the future */
	TH_reset();
	a.timeout = 0;
	H2K_gp->time.last_ticks = 2;
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x12340ULL * resolution,me)) 
		!= (0x12340 * resolution)) FAIL("set timeout return");
	if (a.timeout != 0x12340) FAIL("bad ticks calculation");
	if (H2K_gp->time.timeouts == NULL) FAIL("not put to tree");

	TH_reset();
	mydev[COUNT_LO] = 2;
	mydev[COUNT_HI] = 2;
	mydev[MATCH_LO] = 0;
	mydev[MATCH_HI] = 0;
	H2K_timer_int(0,NULL,0);

	if (mydev[MATCH_LO] <= 0x40000000) {
		printf("new match: 0x%08x\n",mydev[MATCH_LO]);
		FAIL("Didn't set timer for future");
	}
	if (H2K_gp->time.last_ticks == 0) FAIL("Didn't update last_ticks");

	/* Test some cases that should invalidate active timer: BIGBANG and
	 * FOREVER
	 */
	TH_reset();
	a.timeout = 0;
	H2K_gp->time.last_ticks = 2;

	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x12340ULL * resolution,me)) 
		!= (0x12340 * resolution)) FAIL("set timeout return");
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x0ULL,me)) != 0) FAIL("set bigbang return");
	if (H2K_gp->time.timeouts != NULL) FAIL("didn't clear out of timeout tree");
	if (a.timeout != 0) FAIL("didn't clear timeout");

	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,0x12340ULL * resolution,me)) 
		!= (0x12340 * resolution)) FAIL("set timeout return");
	if ((ret = H2K_timer_trap(H2K_TIMER_TRAP_SET_TIMEOUT,~0x0ULL,me)) != 0) FAIL("set forever return");
	if (H2K_gp->time.timeouts != NULL) FAIL("didn't clear out of timeout tree");
	if (a.timeout != 0) FAIL("didn't clear timeout");

	/* Test timer interrupt */

	TH_reset();
	a.timeout = 0x12345;
	mydev[COUNT_LO] = 2;
	mydev[COUNT_HI] = 0;
	mydev[MATCH_LO] = 0;
	mydev[MATCH_HI] = 0;
	H2K_gp->time.timeouts = &a.tree;
	H2K_timer_int(0,NULL,0);

	if (mydev[MATCH_LO] != 0x12345) {
		printf("new match: 0x%08x\n",mydev[MATCH_LO]);
		FAIL("Didn't set timer for next interrupt");
	}
	if (H2K_gp->time.last_ticks == 0) FAIL("didn't update last_ticks");
	if (H2K_gp->time.timeouts != &a.tree) FAIL("erased unexpired timeout");

	mydev[COUNT_LO] = 0x12345;
	TH_expected_cpuint_post = 1;
	H2K_timer_int(0,NULL,0);
	if (TH_expected_cpuint_post != 0) FAIL("Didn't call cpuint");
	if (H2K_gp->time.timeouts != NULL) FAIL("Didn't empty timeout tree");
	if (H2K_gp->time.last_ticks != 0x12345) FAIL("Didn't update last_ticks");
	if (mydev[MATCH_LO] == 0x12345) FAIL("didn't set timer for next interrupt");

	H2K_gp->time.timeouts = &a.tree;
	H2K_gp->l2_int_base = intdev;
	H2K_gp->l2_ack_base = intdev;
	mydev[MATCH_LO] = 0x12345;
	mydev[MATCH_HI] = 0x12345;
	mydev[COUNT_LO] = 0x0;
	mydev[COUNT_HI] = 0x0;
	mydev[ENABLE] = 0x0;
	mydev[3] = 0x0;
	H2K_timer_init();
	if (mydev[MATCH_LO] == 0x12345) FAIL("init/match");
	if (mydev[MATCH_HI] == 0x12345) FAIL("init/match");
	if (mydev[COUNT_LO] != 0x0) FAIL("init/count");
	if (mydev[COUNT_HI] != 0x0) FAIL("init/count");
	if (mydev[ENABLE] != 0x1) FAIL("init/enable");
	/* FIXME: CHECK OTHER STUFF FOR INIT */
	if (H2K_gp->time.timeouts != NULL) FAIL("init/timeouts");
	if (H2K_gp->time.last_ticks != 0) FAIL("ticks");

	H2K_gp->time.devptr = NULL;
	H2K_timer_init();
	if (H2K_gp->time.devptr != ((void *)TIMER_BASE_VA)) FAIL("reset devptr");
	H2K_gp->time.devptr = mydev;

	puts("TEST PASSED\n");
	return 0;
}
#endif

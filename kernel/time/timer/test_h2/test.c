/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <cputime.h>
#include <context.h>
#include <max.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024*32)
#define STACK_SIZE 128

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

volatile int saw_interrupt = 0;

extern u64_t h2_time_get_time();
extern u64_t h2_time_set_timeout(u64_t timeout);
extern void set_vectors();

volatile unsigned int *timerbase = (void *)0xFFEA0000;
volatile unsigned int *l2vicbase = (void *)0xFFE90000;

int main() 
{
	int i;
	u64_t start,end,end2;
	float delta;
	h2_init(NULL);
	set_vectors();
	h2_vmtrap_setie(1);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN,12,0);
	start = h2_time_get_time();
	for (i = 0; i < SPINS; i++) asm volatile ("nop");
	end = h2_time_get_time();
	if (start == end) FAIL("ticks not advancing");
	printf("Yay, end=0x%016llx\n",end);
	h2_time_set_timeout(end*2);
	for (i = 0; i < SPINS*4; i++) {
		if (saw_interrupt != 0) break;
	}
	if (saw_interrupt == 0) {
		printf("Time now 0x%016llx\n",h2_time_get_time());
		printf("*** TIMER DEBUG DUMP:\n");
		for (i = 0; i < 32; i++) {
			if ((i & 7) == 0) printf("\n%04x:",i*4);
			printf("%08x ",timerbase[i]);
		}
		for (i = 0; i < 32; i++) {
			if ((i & 7) == 0) printf("\n%04x:",(1024+i)*4);
			printf("%08x ",timerbase[1024+i]);
		}
		printf("\n*** L2VIC DEBUG DUMP:\n");
		for (i = 0; i < 4; i++) {
			printf("Slice %d: TYPE: %08x POL: %08x EN: %08x STAT: %08x PEND: %08x\n",i,
				l2vicbase[i+(0x280/4)],
				l2vicbase[i+(0x300/4)],
				l2vicbase[i+(0x100/4)],
				l2vicbase[i+(0x380/4)],
				l2vicbase[i+(0x500/4)]);
		}
		printf("\n");
		FAIL("Didn't see interrupt");
	}
	end2 = h2_time_get_time();
	printf("Time is now 0x%016llx, requested 0x%016llx.\n",end2,2*end);
	printf("  Delta from request=0x%016llx / %lld nsecs\n",end2-2*end,end2-2*end);
	delta = (end2 - 2*end);
	delta *= .0192;
	printf("  approx %f ticks\n",delta);

	h2_vmtrap_setie(1);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN,12,0);
	saw_interrupt = 0;

	end2 = h2_time_get_time();
	end2 += end;
	h2_time_set_timeout(end2);

	for (i = 0; i < SPINS*4; i++) {
		if (saw_interrupt != 0) break;
	}
	if (saw_interrupt == 0) {
		printf("Time now 0x%016llx\n",h2_time_get_time());
		FAIL("Didn't see interrupt");
	}

	puts("TEST PASSED\n");
	return 0;
}


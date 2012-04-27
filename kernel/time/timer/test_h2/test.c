/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#define SPINS (1024*10)
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

int main() 
{
	int i;
	u64_t start,end;
	h2_init(NULL);
	set_vectors();
	start = h2_time_get_time();
	for (i = 0; i < SPINS; i++) asm volatile ("nop");
	end = h2_time_get_time();
	if (start == end) FAIL("ticks not advancing");
	h2_time_set_timeout(end*2);
	for (i = 0; i < SPINS*4; i++) {
		if (saw_interrupt != 0) break;
	}
	if (saw_interrupt == 0) FAIL("Didn't see interrupt");
	return 0;
}


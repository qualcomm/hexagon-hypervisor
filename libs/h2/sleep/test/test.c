/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

#define STACKSIZE 1024
#define SLEEP (1000*1000*5)
#define COUNT_MAX SLEEP
#define DELTA_MIN SLEEP
#define DELTA_MAX (SLEEP + SLEEP / 64)

unsigned long stack[STACKSIZE];

int volatile flag = 0;
unsigned long long int delta = 0;

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

void sleeper() {
	unsigned long long int start_time, end_time;

	printf("Time: %016llx\n", start_time = h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME,0));
	h2_nanosleep(SLEEP);
	printf("Time: %016llx\n", end_time = h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME,0));

	delta = end_time - start_time;
	flag = 1;
	h2_thread_stop(0);
}

int main() 
{
	int i;

	h2_handle_errors(0);

	printf("Expect %d < delta < %d\n", DELTA_MIN, DELTA_MAX);

	h2_thread_create(sleeper, &stack[STACKSIZE], NULL, 0);

	while ( 0 == flag && i < COUNT_MAX) i++;

	printf("delta %lld\n", delta);
	if (DELTA_MIN < delta && delta < DELTA_MAX) {
		printf("TEST PASSED\n");
	} else {
		FAIL("Bad delta\n");
	}
	return(0);
}

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

void FAIL(const char *str)
{
        h2_printf(str);
        exit(1);
}

int main() 
{
	h2_handle_errors(0);
	printf("Time: %016llx\n",h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME,0));
	h2_nanosleep(1000*1000*5);
	printf("Time: %016llx\n",h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME,0));
	printf("TEST PASSED\n");
	return(0);
}

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

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

//SPINS is a delay
#define SPINS (1024*1024)

//OVERHEAD is how long it takes to return from trap, add, and trap again.
#if ARCHV == 3
#define PCYCLES_PER_TCYCLE 6
#define OVERHEAD 1024
#elif ARCHV == 4
#define PCYCLES_PER_TCYCLE 3
#define OVERHEAD 1024
#elif ARCHV == 5
#define PCYCLES_PER_TCYCLE 3
#define OVERHEAD 1536
#else
#error define pcycles per tcycle and overhead
#endif

void FAIL(const char *str)
{
	puts("FAIL");
        h2_printf(str);
        exit(1);
}

void delay()
{
        asm volatile (
	" jump 2f\n"
        " .p2align 4\n"
        "2: \n"
        " loop0(1f,%0) \n"
        "1:\n"
        " { nop; }:endloop0 \n" : : "r"(SPINS) :"lc0");
}

int main() 
{
	unsigned long long int startpcycles, endpcycles, delta;

	// Real delay
	startpcycles = h2_vmtrap_get_pcycles();
	delay();
	endpcycles = h2_vmtrap_get_pcycles();
	delta = (endpcycles - startpcycles) - (SPINS * PCYCLES_PER_TCYCLE);
	//if (delta < 0) delta = 0;
	if (delta > OVERHEAD * PCYCLES_PER_TCYCLE) FAIL("Unexpected delta based on delay");
	/* h2_printf("start pcycles is %8llu\n", startpcycles); */
	/* h2_printf("end pcycles is   %8llu\n", endpcycles); */

	// Delay by setting pcycles
        h2_vmtrap_set_pcycles(0);
	startpcycles = h2_vmtrap_get_pcycles();
        h2_vmtrap_set_pcycles(SPINS);
	endpcycles = h2_vmtrap_get_pcycles();
	if (startpcycles > OVERHEAD) FAIL("Unexpected delay getting pcycles");
	if (endpcycles < SPINS ) FAIL("Causality violated, see your nearest physisist!");
	/* h2_printf("start pcycles is %8llu\n", startpcycles); */
	/* h2_printf("end pcycles is   %8llu\n", endpcycles); */
	
	info("TEST PASSED\n");
	h2_thread_stop(0);
	return(0);
}

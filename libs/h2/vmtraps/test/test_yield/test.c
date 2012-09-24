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
#ifndef NUM_THREADS
#if ARCHV == 3
#define NUM_THREADS 6
#elif ARCHV == 4
#define NUM_THREADS 3
#elif ARCHV == 5
#define NUM_THREADS 3
#else
#error define NUM_THREADS
#endif
#endif

#define STACKSIZE 512

h2_sem_t donesem;

unsigned long long int stack_yield[STACKSIZE];
unsigned long long int stack_finish[STACKSIZE];

unsigned long long int delaystack[NUM_THREADS-1][STACKSIZE];

void FAIL(const char *str)
{
	puts("FAIL");
        h2_printf(str);
        exit(1);
}

void spin()
{
	asm volatile (
	" jump 2f\n"
	" .p2align 4\n"
	"2: \n"
	" loop0(1f,%0) \n"
	"1:\n"
	" { nop; }:endloop0 \n" : : "r"(SPINS) :"lc0");
}

void delay()
{
	while (1) {
		spin();
		// This will just swap around other delay() threads
		h2_vmtrap_yield();
	}
}

void delay_yield()
{
	int i=1;
	while (i) {
		spin();
		spin();
		// This will let allow_finish() to run
		h2_vmtrap_yield();
		i=0;
	}
	// Only yield once... 
	while (1) {
		spin();
	}
}

void allow_finish()
{
	printf("delay_yield yielded\n");
	h2_sem_up(&donesem);
	h2_thread_stop(0);
}

int main() 
{
	short i;
	h2_sem_init_val(&donesem,0);
	// Start a delat thread for each HW thread.
	for(i=0; i<NUM_THREADS-1; i++) {
		h2_thread_create(delay,&delaystack[i][STACKSIZE],NULL,1); 
	}

	// Start a thread that will yield
	h2_thread_create(delay_yield,&stack_yield[STACKSIZE],NULL,6);
        // Start the thread that will allow the test to finish
	h2_thread_create(allow_finish,&stack_finish[STACKSIZE],NULL,6);

	h2_sem_down(&donesem);
	info("TEST PASSED\n");
	return(0);
}

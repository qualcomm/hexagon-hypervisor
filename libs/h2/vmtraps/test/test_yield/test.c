/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

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

#define STACKSIZE 512

h2_sem_t donesem;

int done = 0;

unsigned long long int stack_yield[STACKSIZE];
unsigned long long int stack_finish[STACKSIZE];

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

void delay(unsigned long arg)
{
	while (!done) {
		spin();
		// This will just swap around other delay() threads
		h2_vmtrap_yield();
	}
	h2_thread_stop(arg + 3);
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
	while (!done) {
		spin();
	}
	h2_thread_stop(2);
}

void allow_finish()
{
	printf("delay_yield yielded\n");
	h2_sem_up(&donesem);
	h2_thread_stop(1);
}

int main() 
{
	unsigned int num_hw_threads = __builtin_popcount(h2_info(INFO_HTHREADS));
	info("Num HW threads: %d\n", num_hw_threads);

	unsigned long long int delaystack[num_hw_threads - 1][STACKSIZE];
	int i;

	h2_handle_errors(0);  // creates timer interrupt handler

	h2_sem_init_val(&donesem,0);
	// Start a delay thread for each HW thread.
	for(i=0; i<num_hw_threads-1; i++) {
		h2_thread_create(delay,&delaystack[i][STACKSIZE],(void *)i,1); 
	}

	// Start a thread that will yield
	h2_thread_create(delay_yield,&stack_yield[STACKSIZE],NULL,6);
	// Start the thread that will allow the test to finish
	h2_thread_create(allow_finish,&stack_finish[STACKSIZE],NULL,6);

	h2_sem_down(&donesem);
	info("TEST PASSED\n");
	done = 1;
	while(h2_vmstatus(VMOP_STATUS_CPUS, VMOP_STATUS_VMIDX_SELF) > 1) {  // wait for other threads to exit
		h2_millisleep(1);
	}

	h2_thread_stop(0);
	return(0);
}

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

#define STACKSIZE 512

unsigned long long int stacks0[STACKSIZE];
unsigned long long int stacks1[STACKSIZE];

h2_sem_t donesem;

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

void stop() {
	h2_thread_stop(0);
}

void ie_return()
{
	unsigned int ie;

	h2_vmtrap_setie(1);
	ie = h2_vmtrap_getie();
	if (ie==0) {
		FAIL("Interrupts should be enabled (ie_return)");
	}
	h2_sem_up(&donesem);
	h2_vmtrap_setregs((unsigned long)stop, 0, 0, 0);  // set gelr to something sane
	h2_vmtrap_return();
}

void id_return()
{
	unsigned int ie;

	h2_vmtrap_setie(0);
	ie = h2_vmtrap_getie();
	if (ie) {
		FAIL("Interrupts should be disabled (id_return)");
	}
	h2_sem_up(&donesem);
	h2_vmtrap_setregs((unsigned long)stop, 0, 0, 0);
	h2_vmtrap_return();
}

int main() {

	unsigned int ie;
	h2_sem_init_val(&donesem,0);

	// Disable Interrupts
	h2_vmtrap_setie(0);
	ie = h2_vmtrap_getie();
	if (ie) {
		FAIL("Interrupts should be disabled (main)");
	}

	h2_thread_create(ie_return,&stacks0[STACKSIZE],NULL,1);
        // Wait for thread to finish.
	h2_sem_down(&donesem);

	ie = h2_vmtrap_getie();
	if (ie) {
		FAIL("Interrupts should still be disabled (main)");
	}

	// Enable Interrupts
	h2_vmtrap_setie(1);
	ie = h2_vmtrap_getie();
	if (ie == 0) {
		FAIL("Interrupts should be enabled (main)");
	}

	h2_thread_create(id_return,&stacks1[STACKSIZE],NULL,1);
        // Wait for thread to finish.
	h2_sem_down(&donesem);

	ie = h2_vmtrap_getie();
	if (ie == 0) {
		FAIL("Interrupts should still be enabled (main)");
	}

	info("TEST PASSED\n");
	h2_thread_stop(1);
	return(0);
}

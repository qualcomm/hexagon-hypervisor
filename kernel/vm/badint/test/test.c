/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * There are several functions to test:
 * 
 * H2K_vm_int_deliver
 * H2K_vm_interrupt_peek
 * H2K_vm_interrupt_get
 * H2K_vm_trap_intop
 * H2K_vmtrap_intop
 * H2K_enable_guest_interrupts
 * H2K_disable_guest_interrupts 
 * H2K_vm_check_interrupts
 * 
 * These functions make use of cpuint/shint/... calls and ops structures
 * 
 * 
 * Here is the plan:
 * + Use cpuint / shint
 * + Set up interrupt scenarios, make sure expected behavior happens
 * 
 */

#define MAX_TEST_THREADS 8
#define MAX_TEST_INTERRUPTS 512

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <badint.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int main()
{
	if (H2K_vm_badint_func(NULL,NULL,0,NULL) != -1) FAIL("badint");
	puts("TEST PASSED");
	return 0;
}


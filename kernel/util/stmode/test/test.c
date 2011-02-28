/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <max.h>
#include <stmode.h>
#include <hw.h>
#include <q6standalone.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

volatile int handshake = 0;

#define STACK_SIZE 16

unsigned long long int stack1[STACK_SIZE];
unsigned long long int stack2[STACK_SIZE];

void test1(void *unused)
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	while (1) {
		handshake = 1;
		asm volatile (" wait(r0)\n");
	}
}

void test2(void *unused)
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	while (1) {
		handshake = 1;
		asm volatile (" nop\n");
	}
}

int main()
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	H2K_set_gie();

	tmp = H2K_stmode_begin();
	if (tmp != 0) FAIL("stmode failed (1 thread active)");
	if ((H2K_get_syscfg() & 0x10) != 0) FAIL("stmode_begin didn't disable gie");
	H2K_stmode_end();
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_end didn't ensable gie");

	thread_create(&test1,stack1+STACK_SIZE,1,(void *)1);
	while (handshake == 0) /* SPIN */;
	for (tmp = 0; tmp < 100; tmp++) { asm volatile ("nop"); }

	tmp = H2K_stmode_begin();
	if (tmp != 0) FAIL("stmode failed (1 thread sleep)");
	if ((H2K_get_syscfg() & 0x10) != 0) FAIL("stmode_begin didn't disable gie");
	H2K_stmode_end();
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_end didn't ensable gie");

	handshake = 0;

	thread_create(&test2,stack2+STACK_SIZE,2,(void *)2);
	while (handshake == 0) /* SPIN */;
	for (tmp = 0; tmp < 100; tmp++) { asm volatile ("nop"); }

	tmp = H2K_stmode_begin();
	if (tmp == 0) FAIL("stmode passed (2 threads active)");
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_begin didn't reenable gie");

	puts("TEST PASSED");
	return 0;
}


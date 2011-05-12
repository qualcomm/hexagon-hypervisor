/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <config.h>
#include <fatal.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

char buf[sizeof(H2K_thread_context)*2] __attribute__((aligned(32)));

static void __attribute__((noreturn)) foo(u32_t xyzzy)
{
	__builtin_trap();
}

int main()
{
	u32_t i,j,pos,ret;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_thread_init();
	H2K_fatal_kernel_handler = NULL;
	/* Bad config value */
	ret = H2K_trap_config(4,buf,sizeof(buf),0,NULL);
	if (ret != 0) FAIL("Bad return value");
	if (H2K_gp->free_threads) FAIL("trap config failure");
	if (H2K_fatal_kernel_handler != NULL) FAIL("trap config failure");

	/* Configure fatal kernel handler */
	ret = H2K_trap_config(1,foo,0,0,NULL);
	if (ret != 0) FAIL("Bad return value");
	if (H2K_fatal_kernel_handler != foo) FAIL("Kernel fatal handler error");

	/* Add two threads */
	memset(buf,0xef,sizeof(buf));
	ret = H2K_trap_config(0,buf,sizeof(buf),0,NULL);
	if (ret != 2) FAIL("Wrong # of threads returned");
	if (H2K_gp->free_threads != (void *)(buf+sizeof(H2K_thread_context))) FAIL("free threads unexpected");
	if (H2K_gp->free_threads->next != (void *)buf) FAIL("Incorrect number of free threads");
	if (H2K_gp->free_threads->next->next != NULL) FAIL("End of list not found");
	for (i = 4; i < sizeof(H2K_thread_context); i++) {
		if (buf[i] != 0) FAIL("Thread not initialized (A) ");
		if (buf[i+sizeof(H2K_thread_context)] != 0) FAIL("Thread not initialized (B) ");
	}
	for (i = 1; i <= sizeof(H2K_thread_context); i++) {
		H2K_thread_init();
		memset(buf,0xef,sizeof(buf));
		/* Storage for 1 thread */
		ret = H2K_trap_config(0,buf+i,sizeof(buf)-i,0,NULL);
		if (ret != 1) FAIL("Wrong # of threads returned");
		pos = ((i + 31) & (-32));
		if (H2K_gp->free_threads != (void *)(buf+pos)) FAIL("Incorrect start for thread");
		if (H2K_gp->free_threads->next != NULL) FAIL("Incorrect size calculation");
		for (j = pos+4; j < pos+sizeof(H2K_thread_context); j++) {
			if (buf[j] != 0) FAIL("thread not initialized");
		}
		for (j = pos+sizeof(H2K_thread_context); j < sizeof(buf); j++) {
			if (buf[j] != 0xef) FAIL("Overwrote memory");
		}
	}
	for (i = sizeof(H2K_thread_context)+1; i <= sizeof(buf); i++) {
		H2K_thread_init();
		memset(buf,0xef,sizeof(buf));
		/* Storage for 0 threads */
		ret = H2K_trap_config(0,buf+i,sizeof(buf)-i,0,NULL);
		if (ret != 0) FAIL("Wrong # of threads returned");
		if (H2K_gp->free_threads != NULL) FAIL("Insufficient size allocated thread");
		for (j = 0; j < sizeof(buf); j++) {
			if (buf[j] != 0xef) FAIL("Should not write memory");
		}
	}
	H2K_thread_init();
	memset(buf,0xef,sizeof(buf));
	/* Storage for threads, but bad alignment */
	ret = H2K_trap_config(0,buf+1,sizeof(H2K_thread_context),0,NULL);
	if (ret != 0) FAIL("Wrong # of threads");
	if (H2K_gp->free_threads != NULL) FAIL("Insufficient size allocated thread");
	for (j = 0; j < sizeof(buf); j++) {
		if (buf[j] != 0xef) FAIL("Should not write memory");
	}
	puts("TEST PASSED\n");
	return 0;
}


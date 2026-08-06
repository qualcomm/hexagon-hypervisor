/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>

/* FIXME: need to check that we actually dumped the right state.  Also need to check h2_handle_errors(1), i.e. check that we call exit(). */

int main() {

	unsigned int ssr;
	asm volatile ("%0 = ssr \n" : "=r"(ssr));
	printf("ssr=0x%08x ssr_um=%d\n", ssr, (ssr >> 16) & 1);

	// h2_handle_errors(0);

	// do something bad - trap1 with invalid number triggers H2K_vmtrap_bad -> H2K_vm_event
	printf("about to do bad trap\n");
	asm volatile ("trap1(#7) \n");

	/* Shouldn't reach here */
	puts("FAIL\n");
	h2_thread_stop(1);
	return 0;
}


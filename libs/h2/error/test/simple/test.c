/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <h2.h>

/* FIXME: need to check that we actually dumped the right state.  Also need to check h2_handle_errors(1), i.e. check that we call exit(). */

int main() {

	//	volatile int *p = 0;
	unsigned int x;

	h2_handle_errors(0);

	// do something bad
	//	*p++;
	asm volatile ("%0 = ssr \n" : "=r"(x));

	puts("TEST PASSED\n");
	h2_thread_stop(0);
	return 0;
}


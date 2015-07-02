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

	/* Print this first since we don't get control back if the error is handled
		 correctly. Will print FAIL if execution continues after error, which will
		 override the pass signature */
	puts("TEST PASSED\n");

	// do something bad
	//	*p++;
	asm volatile ("%0 = ssr \n" : "=r"(x));

	/* Shouldn't reach here */
	puts("FAIL\n");
	h2_thread_stop(1);
	return 0;
}


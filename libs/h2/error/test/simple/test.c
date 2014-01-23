/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
int main() {

	volatile int *p = 0;

	h2_handle_errors();

	// do something bad
	*p++;

	puts("TEST PASSED\n");
	h2_thread_stop(0);
	return 0;
}


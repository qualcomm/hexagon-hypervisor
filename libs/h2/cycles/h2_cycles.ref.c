/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>

#define QUOTE(X) #X
#define STR(X) QUOTE(X)

unsigned long long int h2_get_pcycles(void) {

	unsigned long long int ret;

	asm volatile (" trap0(#"STR(H2_TRAP_CPUTIME)") \n"
								" %0 = r1:0 \n"
								: "=r"(ret) : : "r0","r1","r2","r3","r4","r5","r6","r7","memory");
	return ret;
}


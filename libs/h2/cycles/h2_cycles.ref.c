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

unsigned long long int h2_get_core_pcycles(void) {

	unsigned long long int ret;

#if ARCHV < 60
	asm volatile (" trap0(#"STR(H2_TRAP_GET_PCYCLES)") \n"
								" %0 = r1:0 \n"
								: "=r"(ret) : : "r0","r1","r2","r3","r4","r5","r6","r7","memory");
#else
	asm volatile ( " %0 = c15:14 // READ UPCYCLES \n" : "=r"(ret));
#endif

	return ret;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_alloc.h>
#include <h2_common_defs.h>
#include <h2_trap_constants.h>

unsigned int h2_prof_sample(unsigned long long int **res) {

	unsigned int hthreads_mask;

	if (NULL == (*res = (unsigned long long int *)h2_malloc(sizeof(unsigned long long int) * 6))) {
		return NULL;
	}

	asm volatile
		(" trap0(#13)\n"
		 " %0 = r0\n"
		 " %1 = r3:2\n"
		 " %2 = r5:4\n"
		 " %3 = r7:6\n"
		 " %4 = r9:8\n"
		 " %5 = r11:10\n"
		 " %6 = r13:12\n"
		 : "=&r"(hthreads_mask), "=&r"((*res)[0]), "=&r"((*res)[1]), "=&r"((*res)[2]), "=&r"((*res)[3]), "=&r"((*res)[4]), "=&r"((*res)[5])
		 :
		 : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13");

	if (0 == hthreads_mask) {
		h2_free(*res);
	}

	return hthreads_mask;
}


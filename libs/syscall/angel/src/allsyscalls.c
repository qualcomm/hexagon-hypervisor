/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <angel.h>
#include <string.h>
#include <stdlib.h>
#include <syscall_defs.h>
#include <h2_thread.h>

#define ALIGN32_DOWN(X) ((X) & -32)
#define ALIGN32_UP(X) (((X) + 31) & -32)

long __boot_net_phys_offset__;

unsigned int angel(unsigned int r0, void *r1, unsigned int r2) {
	return __angel(r0, ANGEL_OFFSET_PTR(r1), r2);
}

void clean(const void *vx,int words)
{
	const int *x = vx;
	int i;
	for (i = 0; i < words; i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	};
	asm volatile (" syncht ");
}

/* X must be 32-byte aligned and COUNT must be a multiple of 32. */
void invalidate(const char *x,count_t count)
{
	count_t i;
	for (i = 0; i < count; i += 32) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	};
	asm volatile (" syncht ");
}

void clean_str(const char *x)
{
	int len = strlen(x);
	int i;
	for (i = 0; i <= (len+1); i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	}
	asm volatile (" syncht ");
}

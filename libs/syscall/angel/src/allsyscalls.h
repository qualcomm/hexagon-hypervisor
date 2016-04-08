/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ALLSYSCALLS_H
#define H2_ALLSYSCALLS_H 1
#include <angel.h>
#include <string.h>
#include <stdlib.h>

#define ALIGN32_DOWN(X) ((X) & -32)
#define ALIGN32_UP(X) (((X) + 31) & -32)

unsigned int angel(unsigned int r0, void *r1, unsigned int r2);

static inline void clean(const void *vx,int words)
{
	const int *x = vx;
	int i;
	for (i = 0; i < words; i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	};
	asm volatile (" syncht ");
}

/* X must be 32-byte aligned and COUNT must be a multiple of 32. */
static inline void invalidate(const char *x,count_t count)
{
	count_t i;
	for (i = 0; i < count; i += 32) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	};
	asm volatile (" syncht ");
}

static inline void clean_str(const char *x)
{
	int len = strlen(x);
	int i;
	for (i = 0; i <= (len+1); i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	}
	asm volatile (" syncht ");
}

#endif

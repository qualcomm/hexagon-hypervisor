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

void clean(const void *vx,int words);

/* X must be 32-byte aligned and COUNT must be a multiple of 32. */
void invalidate(const char *x,count_t count);

void clean_str(const char *x);

#endif

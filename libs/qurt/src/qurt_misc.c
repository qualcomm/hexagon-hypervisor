/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>

unsigned int QURT_MAX_HTHREADS __attribute__((section(".sdata"))) = 6;
unsigned int QURTK_MAX_HTHREADS __attribute__((section(".sdata"))) = 6;

extern char start;
unsigned int image_vstart = (int)(&start);
unsigned int image_pstart = (int)(&start);

/* If you want to cheat, I'll cheat too */

unsigned long long int qurtos_mmap_table[1] = { 0ULL };

void qurt_bad_symbol() {
	UNSUPPORTED;
}


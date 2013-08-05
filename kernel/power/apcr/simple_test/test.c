/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <hw.h>
//#include <globals.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

#ifndef DEBUG
#define ITERS 1
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 32
#endif
#else
#define ITERS 2
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 30
#endif
#endif

#define PASSFAIL_VA 0x01000000

int main() 
{
	int i;
	u32_t *sigil = (void *)(PASSFAIL_VA);

	for (i = 0; i < ITERS; i++) {
		h2_intwait(INTERRUPT_NUM);

#ifdef DEBUG
		*sigil = i + 1;
#endif
	}
	*sigil = 0xe0f0beef;

	h2_thread_stop(0);
	return 0;
}


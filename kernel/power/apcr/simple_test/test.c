/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_cache.h>
#include <h2_intwait.h>
#include <h2_thread.h>

#ifdef PRIME_TLB
#include <tlbfmt.h>
#endif

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
/* assume identity mapping */
#define PASSFAIL_PA PASSFAIL_VA

int main() 
{

#ifdef PRIME_TLB

	H2K_mem_tlbfmt_t pte;
	int idx = 9;

	pte.raw = 0;
	pte.ppd = ((PASSFAIL_PA >> 12) << 1) | 1;
	pte.cccc = 0x6;
	pte.xwru = 0xf;
	pte.vpn = (PASSFAIL_VA >> 12);
	pte.global = 1;
	pte.valid = 1;

	asm volatile
		(
		 " tlbw(%0, %1)\n"
		 " isync\n"
		 :
		 : "r"(pte.raw), "r"(idx)
		 : "memory"
		 );

#endif

	int i;
	unsigned int *sigil = (void *)(PASSFAIL_VA);

	for (i = 0; i < ITERS; i++) {
		h2_intwait(INTERRUPT_NUM);

#ifdef DEBUG
		*sigil = i + 1;
#endif
	}
	*sigil = 0xe0f0beef;
	h2_dccleana(sigil);
	h2_thread_stop(0);
	return 0;
}


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

#define INTERRUPT_NUM 32
#define ITERS 1

#define PASSFAIL_PA 0x01000000
#define PASSFAIL_VA 0xE0000000

int main() 
{
	int i;
	H2K_mem_tlbfmt_t pte;
	u32_t *sigil = (void *)(PASSFAIL_VA);

	pte.raw = 0;
	pte.ppd = ((PASSFAIL_PA >> 12) << 1) | 1;
	pte.cccc = 0x6;
	pte.xwru = 0xf;
	pte.vpn = (PASSFAIL_VA >> 12);
	pte.global = 1;
	pte.valid = 1;

	H2K_mem_tlb_write(9,pte.raw);
	asm volatile ("syncht");

	//*sigil = 0xe0f0beef;
	for (i = 0; i < ITERS; i++) {
		h2_intwait(INTERRUPT_NUM);
		//*sigil = 0xe0f0beef;
	}
	*sigil = 0xe0f0beef;
	//*sigil = 0xe0f0dead;
	h2_thread_stop(0);
	return 0;
}


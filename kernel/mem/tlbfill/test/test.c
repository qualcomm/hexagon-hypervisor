/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <tlbfmt.h>
#include <tlbfill.h>
#include <cfg_table.h>
#include <hw.h>
#include <max.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

/* Helper functions */

void TH_mem_tlb_fill(u32_t va, H2K_thread_context* me)
{
    H2K_mem_tlb_fill(va, me);
}

int main() 
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_kg_init(0,0,125,128);
	H2K_kg.stlbptr = NULL;

    u32_t va = H2K_LINK_ADDR;  // VA set H2K_LINK_ADDR OK to use if just within a unit test 

    H2K_thread_context H2K_tc;

    volatile u32_t ccr = 0; // CCR set for running thread
    __asm__ __volatile(
        " %0 = ccr \n"
        : "=r"(ccr));
    H2K_tc.ccr = ccr;

    volatile u32_t ssr = 0; // SSR set for running thread
    __asm__ __volatile(
        " %0 = ssr \n"
        : "=r"(ssr));
    H2K_tc.ssr = ssr;

    // TH_mem_tlb_fill(va,&H2K_tc); // WIP as ultimately ends in call to thread-stop

	puts("TEST PASSED\n");
	return 0;
}


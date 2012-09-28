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
#include <h2.h>
#include <setjmp.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

jmp_buf env;
void TH_do_debug();
void TH_usermode();

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int TH_saw_debug = 0;

/*
 * Check the error that was called 
 * If we switched stacks, set TH_stack_switch to 1.
 * Return via longjmp
 */
void TH_debug_check(int pass, u32_t sp)
{
	TH_saw_debug = 1;
	longjmp(env,1);
}

extern void set_vectors();

void TH_call_debug()
{
	if (setjmp(env) == 0) {
		TH_do_debug();
	}
}

int main() 
{
	u32_t asid;
	h2_init(NULL);
	set_vectors();
	asm volatile (
	" %0 = ssr \n"
	" %0 = extractu(%0,#7,#8)\n" 
	: "=r"(asid));
	u32_t tlb_index = H2K_mem_tlb_probe(H2K_LINK_ADDR, asid);
	if (tlb_index == 0x80000000) {
		FAIL("Can't find monitor TLB entry");
	}
	u64_t tlb_entry = H2K_mem_tlb_read(tlb_index);
#if __QDSP6_ARCH__ <= 3
	tlb_entry |= 0x7ULL << 29;
#else
	tlb_entry |= 0xfULL << 28;
#endif
	H2K_mem_tlb_write(tlb_index, tlb_entry);

	TH_usermode();

	TH_call_debug();
	if (TH_saw_debug == 0) FAIL("no debug");

	puts("TEST PASSED\n");
	return 0;
}


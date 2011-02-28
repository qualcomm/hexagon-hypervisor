/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <asm_offsets.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <intconfig.h>
#include <setjmp.h>
#include <max.h>
#include <globals.h>

H2K_thread_context *TH_me;
u32_t TH_intno;
s32_t TH_secondary_int_expected;
H2K_thread_context a,b;
u32_t TH_fastint_mask;

extern char _SDA_BASE_;

jmp_buf env;
jmp_buf env2;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void TH_call_fastint_check(u32_t intno);
void TH_call_fastint_intpending(u32_t intno, H2K_thread_context *me, u32_t int2);

void TH_fastint_call(u32_t intno, H2K_thread_context *me, u32_t hthread);

void H2K_switch(H2K_thread_context *from, H2K_thread_context *to)
{
	if (from != NULL) FAIL("Unexpected FROM");
	if (to != NULL) FAIL("Unexpected TO");
	longjmp(env,1);
}

void TH_good_interrupt(u32_t intno)
{
	u32_t sp_check;
	u32_t sgp_check;
	if (intno != TH_intno) FAIL("Unexpected interrupt");
	asm ( " %0 = r29 " : "=r"(sp_check));
	if ((u32_t)(sp_check - ((u32_t)(&H2K_fastint_contexts[0]))) > ((u32_t)FASTINT_CONTEXT_SIZE)) {
		FAIL("Not fastint context sp");
	}
#if __QDSP6_ARCH__ <= 3
	asm ( " %0 = sgp " : "=r"(sgp_check));
#else
	asm ( " %0 = sgp0 " : "=r"(sgp_check));
#endif
	if ((u32_t)(sgp_check) != ((u32_t)(&H2K_fastint_contexts[0]))) {
		FAIL("Not fastint context sgp");
	}
}

void TH_bad_interrupt(u32_t intno)
{
	FAIL("Wrong interrupt called");
}

void TH_setup_fastinthandlers(u32_t interrupt)
{
	u32_t i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->fastint_funcptrs[i] = TH_bad_interrupt;
	}
	H2K_gp->fastint_funcptrs[interrupt] = TH_good_interrupt;
	H2K_gp->fastint_mask = TH_fastint_mask;
}

void TH_fastint_wrapper(u32_t interrupt, H2K_thread_context *dest, u32_t hthread)
{
	TH_fastint_call(interrupt,dest,0);
	longjmp(env2,1);
}

void TH_do_fastint(H2K_thread_context *dest, u32_t interrupt)
{
	TH_intno = interrupt;
	TH_secondary_int_expected = -1;
	TH_setup_fastinthandlers(interrupt);
	if (setjmp(env2) == 0) {
		TH_fastint_wrapper(interrupt,dest,0);
	}
}

void TH_do_fastint_check(u32_t interrupt)
{
	TH_intno = interrupt;
	TH_secondary_int_expected = -1;
	TH_setup_fastinthandlers(interrupt);
	TH_call_fastint_check(interrupt);
}

void TH_do_fastint_intpending(H2K_thread_context *dest, u32_t interrupt, u32_t int2)
{
	TH_intno = interrupt;
	TH_setup_fastinthandlers(interrupt);
	if (H2K_gp->fastint_mask & (1<<int2)) {
		TH_secondary_int_expected = int2;
	} else {
		TH_secondary_int_expected = -1;
	}
	if (setjmp(env) == 0) {
		TH_call_fastint_intpending(interrupt,dest,int2);
	}
	if (TH_secondary_int_expected != -1) FAIL("Didn't take expected interrupt");
}

void TH_saw_secondary_int(u32_t intno)
{
	if (TH_secondary_int_expected != intno) {
		FAIL("Secondary int");
	}
	TH_secondary_int_expected = -1;
	longjmp(env,1);
}

int main() 
{
	int i,j;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_gp->fastint_gp = (u32_t)(&_SDA_BASE_);
	TH_fastint_mask = 0xffffffffU;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint(&a,i);
		TH_do_fastint(NULL,i);
	}
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint_check(i);
		for (j = 0; j < MAX_INTERRUPTS; j++) {
			TH_do_fastint_intpending(&a,i,j);
			TH_do_fastint_intpending(NULL,i,j);
		}
	}
	TH_fastint_mask = 0xffcc5500;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint(&a,i);
		TH_do_fastint(NULL,i);
	}
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint_check(i);
		for (j = 0; j < MAX_INTERRUPTS; j++) {
			TH_do_fastint_intpending(&a,i,j);
			TH_do_fastint_intpending(NULL,i,j);
		}
	}
	puts("TEST PASSED\n");
	return 0;
}


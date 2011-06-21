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

extern char _SDA_BASE_;

jmp_buf env;
jmp_buf env2;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

//void TH_call_fastint_check(u32_t intno);
void TH_call_fastint_intpending(u32_t intno, H2K_thread_context *me, u32_t int2);

void TH_fastint_call(u32_t intno, H2K_thread_context *me, u32_t hthread);

void H2K_switch(H2K_thread_context *from, H2K_thread_context *to)
{
	if (from != NULL) FAIL("Unexpected FROM");
	if (to != NULL) FAIL("Unexpected TO");
	longjmp(env,1);
}

int TH_good_interrupt(u32_t intno)
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
	return 0;
}

int TH_good_interrupt_ackl2(u32_t intno)
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
	return 1;
}

int TH_bad_interrupt(u32_t intno)
{
	FAIL("Wrong interrupt called");
	return 0;
}

void TH_setup_fastinthandlers(u32_t interrupt)
{
	u32_t i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->fastint_funcptrs[i] = TH_bad_interrupt;
	}
	H2K_gp->fastint_funcptrs[interrupt] = TH_good_interrupt;
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

#if 0
void TH_do_fastint_check(u32_t interrupt)
{
	TH_intno = interrupt;
	TH_secondary_int_expected = -1;
	TH_setup_fastinthandlers(interrupt);
	TH_call_fastint_check(interrupt);
}
#endif

#if 0
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
#endif

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
	/* Set up KGP */
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	/* Set up fast interrupt gp */
	H2K_gp->fastint_gp = (u32_t)(&_SDA_BASE_);
	/* Try fast interrupts */
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint(&a,i);
		TH_do_fastint(NULL,i);
	}
#if 0
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint_check(i);
		for (j = 0; j < MAX_INTERRUPTS; j++) {
			TH_do_fastint_intpending(&a,i,j);
			TH_do_fastint_intpending(NULL,i,j);
		}
	}
#endif
	/* Try fast interrupts when some are disabled */
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint(&a,i);
		TH_do_fastint(NULL,i);
	}
#if 0
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		TH_do_fastint_check(i);
		for (j = 0; j < MAX_INTERRUPTS; j++) {
			TH_do_fastint_intpending(&a,i,j);
			TH_do_fastint_intpending(NULL,i,j);
		}
	}
#endif
	puts("TEST PASSED\n");
	return 0;
}


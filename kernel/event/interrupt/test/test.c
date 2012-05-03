/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <intconfig.h>
#include <setjmp.h>
#include <max.h>
#include <globals.h>

/*
 * Strategy:
 * Fill a thread context with known values: "src".
 * Have a different, empty thread context: "dst".
 * Set up a call to the interrupt handler in various situations
 * -- fill registers with values from "src"
 * -- set SGP to "dst"
 * Check to make sure we got to the right place
 * Check to make sure the values are where they are supposed to be.
 */

H2K_thread_context *TH_src_context;
H2K_thread_context *TH_dest_context;
u32_t TH_intno;
u32_t TH_pass;
u32_t TH_fastint_check;
u32_t TH_saw_continuation;
u32_t TH_saw_preempt_call;

jmp_buf env;

void FAIL(const char *str)
{
	puts(str);
	puts("FAIL");
	exit(1);
}

H2K_thread_context srcdata;
H2K_thread_context a,b;

void H2K_preempt()
{
	TH_saw_preempt_call = 1;
}

void H2K_handle_int();
void H2K_interrupt_restore();

/* Helper function, make SGP safe */
void TH_save_sgp();
void TH_restore_sgp();

/* Helper function, will call H2K_handle_interrupt */
void TH_do_interrupt(H2K_thread_context *src, H2K_thread_context *dest, u32_t num);
void TH_do_preempt(H2K_thread_context *src, H2K_thread_context *dest, u32_t num);

#define CHECK_INTERRUPT_TEST(ELEMENT) \
	if (src->ELEMENT != dest->ELEMENT) FAIL("Not equal: " #ELEMENT)

/* Check to make sure we have the expected values in our thread context */
/* Context save/restore checked pretty well in context, I think */

void TH_check_interrupt(H2K_thread_context *src, H2K_thread_context *dest)
{
	CHECK_INTERRUPT_TEST(r0100);
	CHECK_INTERRUPT_TEST(r0302);
	CHECK_INTERRUPT_TEST(r0504);
	CHECK_INTERRUPT_TEST(r0706);
	CHECK_INTERRUPT_TEST(r0908);
	CHECK_INTERRUPT_TEST(r1110);
	CHECK_INTERRUPT_TEST(r1312);
	CHECK_INTERRUPT_TEST(r1514);
	CHECK_INTERRUPT_TEST(r1716);
	CHECK_INTERRUPT_TEST(r1918);
	CHECK_INTERRUPT_TEST(r2120);
	CHECK_INTERRUPT_TEST(r2322);
	CHECK_INTERRUPT_TEST(r2524);
	CHECK_INTERRUPT_TEST(r2726);
	CHECK_INTERRUPT_TEST(r2928);
	CHECK_INTERRUPT_TEST(r3130);
}

/* 
 * Our implementation of H2K_switch for testing.  It should always
 * be from NULL and to NULL
 */

void H2K_switch(H2K_thread_context *from, H2K_thread_context *to)
{
	if (from != NULL) FAIL("Unexpected FROM");
	if (to != NULL) FAIL("Unexpected TO");
	asm volatile (" k0unlock ");
	longjmp(env,1);
}

/* If TH_fastint_check was set, we clear it and longjmp back.
   Otherwise, we fail, as we didn't expect to have a fastint_check
   actually be called. */
void TH_interrupted_fastint_check()
{
	if (TH_fastint_check == 0) FAIL("Jumped to fastint check");
	TH_fastint_check = 0;
	longjmp(env,1);
}

/* Check to make sure that the interrupt was called correctly */
void TH_good_interrupt(u32_t intno, H2K_thread_context *me, u32_t hwtnum)
{
	if (intno != TH_intno) FAIL("Unexpected interrupt");
	if (me != TH_dest_context) FAIL("Unexpected me pointer");
	if (me != NULL) {
		TH_check_interrupt(TH_src_context, me);
		if (TH_pass == 0) {
			TH_pass = 1;
			me->ssrelr = (me->ssrelr & 0xffffffff00000000ULL)  | ((u32_t)H2K_handle_int);
			return;
		} else {
			longjmp(env,1);
		}
	} else {
		/* We expect return code to switch from NULL to NULL... 
		 * see H2K_switch above */
		return;
	}
}

/* Fail */
void TH_bad_interrupt(u32_t intno, H2K_thread_context *me, u32_t hwtnum)
{
	printf("intno=%d\n",intno);
	FAIL("Wrong interrupt called");
}

/*
 * TH_setup_inthandlers sets up all the inthandler routines as TH_bad_interrupt
 * except for the interrupt we actually wish to be taken 
 */

void TH_setup_inthandlers(u32_t interrupt)
{
	u32_t i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i] = TH_bad_interrupt;
	}
	H2K_gp->inthandlers[interrupt] = TH_good_interrupt;
}

/*
 * The TH_try_interrupt sets up the expected src and dst context,
 * and if setjmp is not returning non-local, calls TH_do_interrupt.
 * When it's done, we make sure SGP is reset to correct value.
 */

void TH_try_interrupt(H2K_thread_context *dest, u32_t interrupt)
{
	H2K_thread_context *src = &srcdata;
	TH_src_context = src;
	TH_dest_context = dest;
	TH_intno = interrupt;
	TH_setup_inthandlers(interrupt);
	TH_pass = 0;
	TH_saw_continuation = 0;
	if (setjmp(env) == 0) {
		TH_do_interrupt(src,dest,interrupt);
	}
	TH_restore_sgp();
	if (TH_saw_continuation != 0) FAIL("Called continuation");
}

void TH_try_preempt_interrupt(H2K_thread_context *dest, u32_t interrupt)
{
	H2K_thread_context *src = dest;
	TH_src_context = dest;
	TH_dest_context = dest;		/* we shouldn't be saving or restoring here */
	TH_intno = interrupt;
	TH_setup_inthandlers(interrupt);
	TH_pass = 0;
	TH_saw_continuation = 0;
	if (setjmp(env) == 0) {
		TH_do_preempt(dest,src,interrupt);
	}
	TH_restore_sgp();
	if (TH_saw_continuation == 0) FAIL("Didn't call continuation");
}

void TH_continuation(u64_t r0100)
{
	if (r0100 != srcdata.r0100) FAIL("srcdata doesn't match parameter");
	TH_saw_continuation = 1;
	longjmp(env,1);
}

void TH_continuation_fail(u64_t r0100)
{
	FAIL("Wrong Continuation!");
}

/*
 * fill_srcdata fills up the srcdata structure 
 * with known values.
 */

void fill_srcdata(int i)
{
	u64_t id = ((u64_t)i);
	id = (id << 60) | (id << 28);
	srcdata.r0100 = 0x0001babe0000beefULL | id;
	srcdata.r0302 = 0x0003babe0002beefULL | id;
	srcdata.r0504 = 0x0005babe0004beefULL | id;
	srcdata.r0706 = 0x0007babe0005beefULL | id;
	srcdata.r0908 = 0x0009babe0008beefULL | id;
	srcdata.r1110 = 0x0011babe0010beefULL | id;
	srcdata.r1312 = 0x0013babe0012beefULL | id;
	srcdata.r1514 = 0x0015babe0014beefULL | id;
	srcdata.r1716 = 0x0017babe0016beefULL | id;
	srcdata.r1918 = 0x0019babe0018beefULL | id;
	srcdata.r2120 = 0x0021babe0020beefULL | id;
	srcdata.r2322 = 0x0023babe0022beefULL | id;
	srcdata.r2524 = 0x0025babe0024beefULL | id;
	srcdata.r2726 = 0x0027babe0025beefULL | id;
	srcdata.r2928 = 0x0029babe0028beefULL | id;
	srcdata.r3130 = 0x0031babe0030beefULL | id;
	srcdata.continuation = TH_continuation_fail;
}

int main() 
{
	int i;
	/* Setup SGP correctly */
	TH_save_sgp();
	/* Set up KGP correctly for direct calls */
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	printf("MAX_INTERRUPTS=%d\n",MAX_INTERRUPTS);
	TH_fastint_check = 0;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		/* For each interrupt, try to do the interrupt */
#if __QDSP6_ARCH__ >= 4
		if (i == 31) continue;
#endif
		printf("i=%d\n",i);
		fill_srcdata(i);
		a.continuation = TH_continuation_fail;
		TH_try_interrupt(&a,i);
		TH_try_interrupt(NULL,i);
		a.continuation = TH_continuation;
		TH_try_preempt_interrupt(&a,i);
		/* Test the case where we were checking for 
		 * another interrupt before return */
		//TH_fastint_check = 1;
		//TH_try_interrupt((void *)(&H2K_fastint_contexts[0]),i);
		//if (TH_fastint_check != 0) FAIL("Didn't jump to fastint check");
	}
	a.continuation = TH_continuation_fail;
	TH_try_interrupt(&a,0);
	a.continuation = TH_continuation;
	TH_try_preempt_interrupt(&a,0);
	puts("TEST PASSED\n");
	return 0;
}


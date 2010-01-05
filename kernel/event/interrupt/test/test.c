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

H2K_thread_context *TH_src_context;
H2K_thread_context *TH_dest_context;
u32_t TH_intno;
u32_t TH_pass;

jmp_buf env;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context srcdata;
H2K_thread_context a,b;

void H2K_handle_int();

/* Helper function, will call H2K_handle_interrupt */
void TH_do_interrupt(H2K_thread_context *src, H2K_thread_context *dest, u32_t num);

#define CHECK_INTERRUPT_TEST(ELEMENT) \
	if (src->ELEMENT != dest->ELEMENT) FAIL("Not equal: " #ELEMENT)

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

void H2K_switch(H2K_thread_context *from, H2K_thread_context *to)
{
	if (from != NULL) FAIL("Unexpected FROM");
	if (to != NULL) FAIL("Unexpected TO");
	longjmp(env,1);
}

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

void TH_bad_interrupt(u32_t intno, H2K_thread_context *me, u32_t hwtnum)
{
	FAIL("Wrong interrupt called");
}

void TH_setup_inthandlers(u32_t interrupt)
{
	u32_t i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_inthandlers[i] = TH_bad_interrupt;
	}
	H2K_inthandlers[interrupt] = TH_good_interrupt;
}

void TH_try_interrupt(H2K_thread_context *dest, u32_t interrupt)
{
	H2K_thread_context *src = &srcdata;
	TH_src_context = src;
	TH_dest_context = dest;
	TH_intno = interrupt;
	TH_setup_inthandlers(interrupt);
	TH_pass = 0;
	if (setjmp(env) == 0) {
		TH_do_interrupt(src,dest,interrupt);
	}
}

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
}

int main() 
{
	int i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		fill_srcdata(i);
		TH_try_interrupt(&a,i);
		TH_try_interrupt(NULL,i);
	}
	TH_try_interrupt(&a,0);
	puts("TEST PASSED\n");
	return 0;
}


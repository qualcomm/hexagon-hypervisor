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

H2K_thread_context *TH_src_context;
H2K_thread_context *TH_dest_context;
u32_t TH_intno;
u32_t TH_pass;
u32_t TH_expected_badva;
u32_t TH_expected_elr;
u32_t TH_saw_tlbfill;

jmp_buf env;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context srcdata;
H2K_thread_context a,b;

/* Helper function, will call H2K_handle_interrupt */
void TH_do_tlbmissx(H2K_thread_context *src, H2K_thread_context *dest, u32_t cause, u32_t badva, u32_t elr);
void TH_do_tlbmissrw(H2K_thread_context *src, H2K_thread_context *dest, u32_t cause, u32_t badva, u32_t elr);

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
	// don't check callee saves CHECK_INTERRUPT_TEST(r1716);
	// don't check callee saves CHECK_INTERRUPT_TEST(r1918);
	// don't check callee saves CHECK_INTERRUPT_TEST(r2120);
	// don't check callee saves CHECK_INTERRUPT_TEST(r2322);
	// don't check callee saves CHECK_INTERRUPT_TEST(r2524);
	// don't check callee saves CHECK_INTERRUPT_TEST(r2726);
	CHECK_INTERRUPT_TEST(r2928);
	CHECK_INTERRUPT_TEST(r3130);
}

void H2K_mem_tlb_fill(u32_t badva, H2K_thread_context *me)
{
	// asm volatile (" ssr = %0 \n" : :"r"(0));
	TH_check_interrupt(TH_src_context,me);
	if (badva != TH_expected_badva) FAIL("Unexpected BADVA");
	if (me->elr != TH_expected_elr) FAIL("Unexpected ELR");
	TH_saw_tlbfill = 1;
	longjmp(env,1);
}

void TH_try_tlbmissx(H2K_thread_context *dest, u32_t cause, u32_t elr, u32_t badva)
{
	H2K_thread_context *src = &srcdata;
	TH_src_context = src;
	TH_dest_context = dest;
	TH_expected_elr = elr;
	TH_saw_tlbfill = 0;
	if (setjmp(env) == 0) {
		TH_do_tlbmissx(src,dest,cause,badva,elr);
	}
	if (TH_saw_tlbfill == 0) FAIL("Didn't call TLB fill");
}

void TH_try_tlbmissrw(H2K_thread_context *dest, u32_t cause, u32_t elr, u32_t badva)
{
	H2K_thread_context *src = &srcdata;
	TH_src_context = src;
	TH_dest_context = dest;
	TH_expected_elr = elr;
	TH_saw_tlbfill = 0;
	if (setjmp(env) == 0) {
		TH_do_tlbmissrw(src,dest,cause,badva,elr);
	}
	if (TH_saw_tlbfill == 0) FAIL("Didn't call TLB fill");
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
	int i = 3;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	fill_srcdata(i);
	/* TLB Miss-X: same page, next page, and icinva */

	puts("a");
	TH_expected_badva = 0x3210bab0;
	TH_try_tlbmissx(&a,0,0x3210bab0,0xdeadbee0);

	puts("b");
	TH_expected_badva = 0x3210bac0;
	TH_try_tlbmissx(&a,1,0x3210bab0,0xdeadbee0);

	puts("c");
	TH_expected_badva = 0xdeadb000;
	TH_try_tlbmissx(&a,2,0x321bab0,0xdeadb000);

	/* TLB Miss-RW: Read, Write */

	puts("d");
	TH_expected_badva = 0xcafebabe;
	TH_try_tlbmissrw(&a,0,0x12345678,0xcafebabe);

	puts("e");
	TH_expected_badva = 0xcafed00d;
	TH_try_tlbmissrw(&a,1,0xbadf00d8,0xcafed00d);

	puts("TEST PASSED\n");
	return 0;
}


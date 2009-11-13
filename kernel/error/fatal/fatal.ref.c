/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <fatal.h>
#include <context.h>
#include <trace.h>
#include <thread_stop.h>

#define SYS_EXCEPTION 0x18

static void doangel(unsigned int what, unsigned int arg, unsigned int r2)
{
	__asm__ __volatile__ (
		" { r0 = %0\n"
		" r1 = %1\n"
		" r2 = %2 }\n"
		" r24 = ssr \n"
		" r4 = clrbit(r24,#16) \n"
		" r4 = clrbit(r4,#17) \n"
		" r4 = clrbit(r4,#18) \n"
		" ssr = r4 \n"
		" trap0(#0) \n"
		" ssr = r24 \n"
		: : "r"(what),"r"(arg),"r"(r2) :"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13","r14","r15","r16","r17","r18","r19","r20","r21","r22","r23","r24","r28");
}

void BLASTK_fatal_kernel(short error_id, BLASTK_thread_context *me, int info0, int info1, int hthread)
{
	BLASTK_trace(error_id, me, info0, info1, hthread);
	doangel(SYS_EXCEPTION,1,1);
}

void BLASTK_fatal_thread(short error_id, BLASTK_thread_context *me, int info0, int info1, int hthread)
{
	BLASTK_trace(error_id, me, info0, info1, hthread);
	return BLASTK_thread_stop(me);
}


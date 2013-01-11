/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <fatal.h>
#include <context.h>
#include <trace.h>
#include <stop.h>
#include <globals.h>
#include <hw.h>

IN_SECTION(".text.misc.fatal")
static void __attribute__((noreturn)) H2K_fatal_sim_exit(u32_t why)
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
		" 1: jump 1b\n"
		: : "r"(0x18),"r"(why),"r"(why) :"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13","r14","r15","r28");
	__builtin_trap();
}

/* void (*H2K_fatal_kernel_handler)(u32_t) __attribute__((noreturn)) = H2K_fatal_sim_exit; */

void H2K_fatal_kernel(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread)
{
	H2K_trace(H2K_TRACE_FATAL_KERNEL,(u32_t)me,H2K_get_pcycle_reg(),hthread);
	H2K_fatal_sim_exit(1);
}

void H2K_fatal_thread(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread)
{
	H2K_trace(H2K_TRACE_FATAL_THREAD,me->id.raw >> 24,H2K_get_pcycle_reg(),hthread);
	return H2K_thread_stop(0xaaafa7a1, me);  // get it?
}


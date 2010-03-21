/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <vmdefs.h>
#include <vmwork.h>
#include <setjmp.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

jmp_buf env;

H2K_thread_context a;

u32_t TH_saw_interrupt_get;
u32_t TH_saw_thread_stop;
u32_t TH_saw_vm_event;

s32_t TH_interrupt_get_retval;

void H2K_vm_event(u32_t badva, u32_t b, u32_t c, H2K_thread_context *d)
{
	if (b != TH_interrupt_get_retval) FAIL("Bad interrupt cause");
	if (c != INTERRUPT_GEVB_OFFSET) FAIL("Bad offset to vm_event");
	if (d != &a) FAIL("Bad context pointer passed");
	TH_saw_vm_event = 1;
}

void H2K_thread_stop(H2K_thread_context *me)
{
	TH_saw_thread_stop = 1;
	longjmp(env,1);
}

s32_t H2K_vm_interrupt_get(void *x, u32_t cpu)
{
	TH_saw_interrupt_get = 1;
	return TH_interrupt_get_retval;
}

void TH_call_vm_do_work(H2K_thread_context *x)
{
	if (setjmp(env) == 0) {
		H2K_vm_do_work(x);
	}
}

int main()
{
	__asm__ __volatile(" r16 = %0\n" : : "r"(&H2K_kg));

	a.vmstatus = H2K_VMSTATUS_KILL;
	TH_call_vm_do_work(&a);
	if (!TH_saw_thread_stop) FAIL("Didn't kill thread");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	a.vmstatus = H2K_VMSTATUS_IE | H2K_VMSTATUS_KILL;
	TH_call_vm_do_work(&a);
	if (!TH_saw_thread_stop) FAIL("Didn't kill thread");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	a.vmstatus = H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_KILL;
	TH_call_vm_do_work(&a);
	if (!TH_saw_thread_stop) FAIL("Didn't kill thread");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	a.vmstatus = H2K_VMSTATUS_VMWORK;
	TH_interrupt_get_retval = -1;
	TH_call_vm_do_work(&a);
	if (TH_saw_thread_stop) FAIL("Killed thread?");
	if (TH_saw_interrupt_get) FAIL("Saw interrupt get while !IE");
	if (TH_saw_vm_event) FAIL("Did a VM event");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	a.vmstatus = H2K_VMSTATUS_IE | H2K_VMSTATUS_VMWORK;
	TH_interrupt_get_retval = -1;
	TH_call_vm_do_work(&a);
	if (TH_saw_thread_stop) FAIL("Killed thread?");
	if (!TH_saw_interrupt_get) FAIL("Didn't see interrupt get");
	if (TH_saw_vm_event) FAIL("Did a VM event");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	a.vmstatus = H2K_VMSTATUS_IE | H2K_VMSTATUS_VMWORK;
	TH_interrupt_get_retval = 0;
	TH_call_vm_do_work(&a);
	if (TH_saw_thread_stop) FAIL("Killed thread?");
	if (!TH_saw_interrupt_get) FAIL("Didn't see interrupt get");
	if (!TH_saw_vm_event) FAIL("Didn't do a VM event / 0");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	a.vmstatus = H2K_VMSTATUS_IE | H2K_VMSTATUS_VMWORK;
	TH_interrupt_get_retval = 1;
	TH_call_vm_do_work(&a);
	if (TH_saw_thread_stop) FAIL("Killed thread?");
	if (!TH_saw_interrupt_get) FAIL("Didn't see interrupt get");
	if (!TH_saw_vm_event) FAIL("Didn't do a VM event / 1");
	TH_saw_thread_stop = TH_saw_vm_event = TH_saw_interrupt_get = 0;

	puts("TEST PASSED");
	return 0;
}


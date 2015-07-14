/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <globals.h>
#include <futex.h>
#include <hw.h>
#include <vmdefs.h>
#include <stdio.h>
#include <stdlib.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

int TH_saw_do_work;
int TH_work_return;
int TH_expected_safemem;

int H2K_vm_do_work(H2K_thread_context *me)
{
	if (me != &a) FAIL("bad context");
	TH_saw_do_work = 1;
	return TH_work_return;
}

u32_t H2K_safemem_check_and_lock(void *va, u32_t perms, pa_t *out, H2K_thread_context *me)
{
	if (me != &a) FAIL("bad context");
	if (TH_expected_safemem == 0) FAIL("unexpected safemem");
	TH_expected_safemem = 0;
	*out = (pa_t)va;
	return 0;
}

int main()
{
	u32_t i = 0;
	a.vmstatus = H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_IE;

	TH_saw_do_work = 0;
	if (H2K_futex_wait(&i,i,&a) != -1) FAIL("return val");
	if (TH_saw_do_work == 0) FAIL("no vmwork");

	TH_saw_do_work = 0;
	if (H2K_futex_lock_pi(&i,&a) != -1) FAIL("pi return val");
	if (TH_saw_do_work == 0) FAIL("no vmwork");

	a.vmstatus = H2K_VMSTATUS_VMWORK;

	TH_saw_do_work = 0;
	TH_expected_safemem = 1;
	if (H2K_futex_wait(&i,i,&a) != -1) FAIL("return val");
	if (TH_saw_do_work != 0) FAIL("vmwork");

	TH_saw_do_work = 0;
	TH_expected_safemem = 1;
	if (H2K_futex_lock_pi(&i,&a) != -1) FAIL("pi return val");
	if (TH_saw_do_work != 0) FAIL("vmwork");

	puts("TEST PASSED\n");
	return 0;
}


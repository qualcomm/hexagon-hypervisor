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

int n_fails = 0;
void CHECK_FAIL(const char *str)
{
	printf("FAIL: %s\n", str);
	n_fails++;
}
#define CHECK(cond, msg) do { if (!(cond)) CHECK_FAIL(msg); } while (0)

H2K_thread_context a;

int TH_saw_do_work;
int TH_work_return;
int TH_expected_safemem;

int H2K_vm_do_work_withlock(H2K_thread_context *me)
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

	/* Case A: VMWORK|IE -- the original gate fires today (and post-fix).
	 * Expected to pass both pre- and post-fix. */
	a.vmstatus = H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_IE;

	TH_saw_do_work = 0;
	CHECK(H2K_futex_wait(&i,i,&a) == -1, "A: wait return val");
	CHECK(TH_saw_do_work != 0, "A: wait did not call vm_do_work");

	TH_saw_do_work = 0;
	CHECK(H2K_futex_lock_pi(&i,&a) == -1, "A: lock_pi return val");
	CHECK(TH_saw_do_work != 0, "A: lock_pi did not call vm_do_work");

	/* Case B: VMWORK alone -- the no-block rule from vmdefs.spec.  Pre-fix
	 * the gate is VMWORK&&IE so falls through to safemem (allowed via
	 * TH_expected_safemem) and returns -1 via the safemem-fail path with
	 * vm_do_work never called.  Post-fix the gate fires on VMWORK alone. */
	a.vmstatus = H2K_VMSTATUS_VMWORK;

	TH_saw_do_work = 0;
	TH_expected_safemem = 1;
	CHECK(H2K_futex_wait(&i,i,&a) == -1, "B: wait return val");
	CHECK(TH_saw_do_work != 0, "B: VMWORK alone must gate (wait)");

	TH_saw_do_work = 0;
	TH_expected_safemem = 1;
	CHECK(H2K_futex_lock_pi(&i,&a) == -1, "B: lock_pi return val");
	CHECK(TH_saw_do_work != 0, "B: VMWORK alone must gate (lock_pi)");

	/* Case C: VMWORK|KILL -- the killed-thread escape from VMKILL_race.md.
	 * kill_thread_locked sets KILL|VMWORK and the victim is on its way into
	 * a blocking syscall.  Pre-fix same failure mode as case B (gate is
	 * VMWORK&&IE; KILL isn't IE). */
	a.vmstatus = H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_KILL;

	TH_saw_do_work = 0;
	TH_expected_safemem = 1;
	CHECK(H2K_futex_wait(&i,i,&a) == -1, "C: wait return val");
	CHECK(TH_saw_do_work != 0, "C: VMWORK|KILL must gate (wait)");

	TH_saw_do_work = 0;
	TH_expected_safemem = 1;
	CHECK(H2K_futex_lock_pi(&i,&a) == -1, "C: lock_pi return val");
	CHECK(TH_saw_do_work != 0, "C: VMWORK|KILL must gate (lock_pi)");

	if (n_fails != 0) {
		puts("FAIL");
		exit(1);
	}
	puts("TEST PASSED\n");
	return 0;
}


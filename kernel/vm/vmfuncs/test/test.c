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
#include <vmfuncs.h>
#include <setjmp.h>
#include <hw.h>
#include <vmdefs.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
H2K_vmblock_t av;

u32_t TH_saw_fatal;

jmp_buf env;

void H2K_fatal_thread()
{
	FAIL("Saw fatal");
}

H2K_kg_t H2K_kg;

u32_t TH_expected_enable = 0;
u32_t TH_enable_ret = 0;

u32_t H2K_enable_guest_interrupts(H2K_thread_context *me)
{
	if (!TH_expected_enable) FAIL("Didn't expect enable");
	TH_expected_enable = 0;
	me->vmstatus |= H2K_VMSTATUS_IE;
	return TH_enable_ret;
}

u32_t TH_expected_disable = 0;
u32_t TH_disable_ret = 0;

u32_t H2K_disable_guest_interrupts(H2K_thread_context *me)
{
	if (!TH_expected_disable) FAIL("Didn't expect disable");
	TH_expected_disable = 0;
	me->vmstatus &= ~H2K_VMSTATUS_IE;
	return TH_disable_ret;
}

u32_t TH_expected_yield = 0;
void H2K_sched_yield(H2K_thread_context *me)
{
	if (!TH_expected_yield) FAIL("Didn't expect disable");
	TH_expected_yield = 0;
}

u32_t TH_expected_stop = 0;
void H2K_thread_stop(H2K_thread_context *me)
{
	if (!TH_expected_stop) FAIL("Didn't expect disable");
	TH_expected_stop = 0;
	longjmp(env,1);
}

u32_t TH_expected_dosched = 0;
void H2K_dosched(H2K_thread_context *me, u32_t hthread)
{
	if (!TH_expected_dosched) FAIL("Didn't expect dosched");
	if (me != &a) FAIL("wrong context");
	if (hthread != me->hthread) FAIL("wrong hthread arg");
	TH_expected_dosched = 0;
	longjmp(env,1);
}

u32_t TH_expected_create = 0;
u32_t TH_create_ret = 0;
u32_t H2K_thread_create_no_squash(u32_t pc, u32_t sp, u32_t arg, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	if (TH_expected_create == 0) FAIL("create: didn't expect create");
	if (vmblock != me->vmblock) FAIL("create: vmblock");
	if (prio != me->base_prio) FAIL("create: prio");
	if (pc != me->r00) FAIL("create: pc");
	if (sp != me->r01) FAIL("create: sp");
	if (arg != 0) FAIL("create: arg");
	TH_expected_create = 0;
	return TH_create_ret;
}

u32_t TH_expected_work = 0;
u32_t TH_work_ret = 0;
u32_t H2K_vm_do_work(H2K_thread_context *me)
{
	if (!TH_expected_work) FAIL("Didn't expect disable");
	TH_expected_work = 0;
	if ((H2K_get_syscfg() & (1<<12)) == 0) FAIL("no k0lock");
	H2K_mutex_unlock_k0();
	return TH_work_ret;
}

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	/* VMVERSION */
	H2K_vmtrap_version(&a);
	if ((a.r00) != H2K_VM_VERSION) FAIL("vmversion");

	/* VMSETVEC */
	a.r00 = 0xabcdef00;
	a.gevb = 0;
	H2K_vmtrap_setvec(&a);
	if (a.r00 != 0) FAIL("setvec/return");
	if (a.gevb != (void *)0xabcdef00U) FAIL("setvec/vec");

	/* VMSETIE */
	a.r00 = 0;
	TH_disable_ret = a.vmstatus = 0;
	TH_expected_disable = 1;
	H2K_vmtrap_setie(&a);
	if (a.r00 != 0) FAIL("setie/0/0/ret");
	TH_expected_disable = 0;				// could be inlined
	if (a.vmstatus != 0) FAIL("setie/0/0/vmstatus");

	a.r00 = 0;
	TH_disable_ret = 1;
	a.vmstatus = H2K_VMSTATUS_IE;
	TH_expected_disable = 1;
	H2K_vmtrap_setie(&a);
	if (a.r00 != 1) FAIL("setie/0/1/ret");
	TH_expected_disable = 0;				// could be inlined
	if (a.vmstatus != 0) FAIL("setie/0/1/vmstatus");

	a.r00 = 1;
	TH_enable_ret = 1;
	a.vmstatus = H2K_VMSTATUS_IE;
	TH_expected_enable = 1;
	H2K_vmtrap_setie(&a);
	if (a.r00 != 1) FAIL("setie/1/1/ret");
	//if (TH_expected_enable) FAIL("Enable not called, maybe that's OK?");
	TH_expected_enable = 0;
	if (a.vmstatus != H2K_VMSTATUS_IE) FAIL("setie/1/1/vmstatus");

	a.r00 = 1;
	TH_enable_ret = a.vmstatus = 0;
	TH_expected_enable = 1;
	H2K_vmtrap_setie(&a);
	if (a.r00 != 0) FAIL("setie/1/0/ret");
	if (TH_expected_enable) FAIL("Enable not called. Probably bad");
	TH_expected_enable = 0;
	if (a.vmstatus != H2K_VMSTATUS_IE) FAIL("setie/1/0/vmstatus");

	/* VMGETIE */
	a.vmstatus = 0;
	H2K_vmtrap_getie(&a);
	if (a.r00 != 0) FAIL("getie/0");

	a.vmstatus = 0xff;
	H2K_vmtrap_getie(&a);
	if (a.r00 != 1) FAIL("getie/f->1");

	/* GETPCYCLES */
	a.totalcycles = 0x8000000000000000ULL;
	H2K_vmtrap_get_pcycles(&a);
	if (a.r0100 <= 0x8000000000000000ULL) FAIL("get totalcycles");

	/* SETPCYCLES */
	a.r0100 = 0;
	H2K_vmtrap_set_pcycles(&a);
	if (a.r0100 != 0x0ULL) FAIL("set totalcycles");

	/* YIELD */
	a.r00 = 0x123;
	TH_expected_yield = 1;
	H2K_vmtrap_yield(&a);
	if (a.r00 != 0) FAIL("yield/ret");
	if (TH_expected_yield) FAIL("yield");

	/* STOP */
	a.r00 = 0x123;
	TH_expected_stop = 1;
	if (setjmp(env) == 0) H2K_vmtrap_stop(&a);
	if (TH_expected_stop) FAIL("stop");

	/* PID */
	a.id.raw = 0x1234abcd;
	H2K_vmtrap_vmpid(&a);
	if (a.r00 != 0x1234abcd) FAIL("vmpid");

	/* SETREGS */
	a.gelr = a.gbadva = a.gssr = a.gosp = 0xdeadbeef;
	a.r0100 = 0x0123456789abcdefULL;
	a.r0302 = 0xfedcba9876543210ULL;
	H2K_gregs_restore(&a);
	H2K_vmtrap_setregs(&a);
	H2K_gregs_save(&a);
	if (a.gelr != 0x89abcdef) FAIL("set/gelr");
	if (a.gssr != 0x01234567) FAIL("set/gssr");
	if (a.gosp != 0x76543210) FAIL("set/gosp");
	if (a.gbadva != 0xfedcba98) FAIL("set/gbadva");

	/* GETREGS */
	a.r0302 = a.r0100 = 0xdeadbeefdeadbeefULL;
	a.gbadva_gosp = 0x0123456789abcdefULL;
	a.gssr_gelr = 0xfedcba9876543210ULL;
	H2K_gregs_restore(&a);
	H2K_vmtrap_getregs(&a);
	H2K_gregs_save(&a);
	if (a.r0302 != 0x0123456789abcdefULL) FAIL("get/r3:2");
	if (a.r0100 != 0xfedcba9876543210ULL) FAIL("get/r1:0");

	/* START */
	TH_expected_create = 1;
	a.r00 = 0x10203040;
	a.r01 = 0x50607080;
	a.base_prio = 4;
	a.vmblock = (void *)&a;
	TH_create_ret = 0x0a0b0c0d;
	H2K_vmtrap_start(&a);
	if (a.r00 != 0x0a0b0c0d) FAIL("start/ret");

	/* WAIT */

	TH_expected_work = 1;
	TH_work_ret = 1;
	a.r00 = 0;
	H2K_vmtrap_wait(&a);
	if (TH_expected_work != 0) FAIL("no work call");
	if (a.r00 != 1) FAIL("didn't return intno");

	TH_expected_work = 1;
	TH_work_ret = -1;
	a.r00 = -1;
	a.vmblock = &av;
	av.waiting_cpus = 0;
	a.id.cpuidx = 5;
	H2K_kg.runlist[0] = &a;
	H2K_kg.runlist_prios[0] = 4;
	a.status = 0;
	TH_expected_dosched = 1;
	if (setjmp(env) == 0) H2K_vmtrap_wait(&a);
	if (TH_expected_dosched != 0) FAIL("didn't dosched");
	if (a.r00 != -1) FAIL("r00 clobbered");
	if (av.waiting_cpus != (1<<5)) FAIL("wrong waiting cpus");
	if (a.status != H2K_STATUS_VMWAIT) FAIL("wrong status");
	if (H2K_kg.runlist[0] != NULL) FAIL("runlist");
	if (H2K_kg.runlist_prios[0] != -1) FAIL("runlist_prios");

	/* RETURN */

	a.gelr = 0xCCCCCCCC;
	a.gssr = 0x00000000;
	a.gosp = 0xdeadbeef;
	a.r29 = 0x44444444;
	H2K_set_elr(0xDEAD0000);
	a.r0100 = 0x1111222233334444ULL;
	a.ssr = ((1<<SSR_GUEST_BIT) | (1<<SSR_SS_BIT));
	H2K_gregs_restore(&a);
	H2K_vmtrap_return(&a);
	H2K_gregs_save(&a);
	if (H2K_get_elr() != 0xCCCCCCCC) FAIL("return/elr");
	if (a.r0100 != 0x1111222233334444ULL) FAIL("return/r0100 clobbered");
	if ((a.ssr & (1<<SSR_SS_BIT)) != 0) FAIL("SS set");
	if ((a.ssr & (1<<SSR_GUEST_BIT)) == 0) FAIL("GUEST not set");
	if (a.gosp != 0xdeadbeef) FAIL("swapped sp");
	if (a.r29 != 0x44444444) FAIL("swapped sp");

	a.gelr = 0xCCCCCCCC;
	a.gssr = 0xE0000000;
	a.gosp = 0xdeadbeef;
	a.r29 = 0x44444444;
	H2K_set_elr(0xDEAD0000);
	a.r0100 = 0x1111222233334444ULL;
	a.ssr = ((1<<SSR_GUEST_BIT) | (0<<SSR_SS_BIT));
	H2K_gregs_restore(&a);
	TH_expected_enable = 1;
	H2K_vmtrap_return(&a);
	H2K_gregs_save(&a);
	if (TH_expected_enable == 1) FAIL("no enable call");
	if (H2K_get_elr() != 0xCCCCCCCC) FAIL("return/elr");
	if (a.r0100 != 0x1111222233334444ULL) FAIL("return/r0100 clobbered");
	if ((a.ssr & (1<<SSR_SS_BIT)) == 0) FAIL("SS not set");
	if ((a.ssr & (1<<SSR_GUEST_BIT)) != 0) FAIL("GUEST set");
	if (a.r29 != 0xdeadbeef) FAIL("didn't swap sp");
	if (a.gosp != 0x44444444) FAIL("didn't swap sp");

	puts("TEST PASSED");
	return 0;
}


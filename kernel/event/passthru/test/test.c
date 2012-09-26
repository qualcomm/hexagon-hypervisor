/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <runlist.h>
#include <lowprio.h>
#include <context.h>
#include <hw.h>
#include <popup.h>
#include <stdio.h>
#include <stdlib.h>
#include <checker_kernel_locked.h>
#include <checker_runlist.h>
#include <checker_ready.h>
#include <setjmp.h>
#include <globals.h>
#include <passthru.h>
#include <vmint.h>
#include <string.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_vmblock_t TH_vmblock;
H2K_thread_context a;
H2K_vm_int_opinfo_t TH_intinfo[3];

H2K_kg_t H2K_kg;

int TH_saw_int = 0;
int TH_intno = 0;

s32_t TH_bad(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, struct H2K_vm_int_opinfo_struct *info)
{
	FAIL("bad op");
	return 0;
}

s32_t TH_good(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, struct H2K_vm_int_opinfo_struct *info)
{
	if (vmblock != &TH_vmblock) FAIL("bad vmblock");
	if (me != &a) FAIL("bad thread");
	if (intno != TH_intno) FAIL("bad interrupt");
	TH_saw_int = 1;
	return 0;
}

const H2K_vm_int_ops_t TH_ops = {
	.nop = TH_bad,
	.enable = TH_bad,
	.disable = TH_bad,
	.localen = TH_bad,
	.localdis = TH_bad,
	.setaffinity = TH_bad,
	.get = TH_bad,
	.peek = TH_bad,
	.status = TH_bad,
	.post = TH_good,
	.clear = TH_bad,
};

void TH_init_vmblock()
{
	memset(&TH_vmblock,0,sizeof(TH_vmblock));
	TH_vmblock.contexts = &a;
	TH_vmblock.max_cpus = 1;
	TH_vmblock.num_ints = 31;
	TH_vmblock.intinfo = TH_intinfo;
	H2K_kg.vmblocks[2] = &TH_vmblock;
	a.vmblock = &TH_vmblock;
	a.id.cpuidx = 0;
	a.id.vmidx = 2;
	TH_intinfo[0].num_ints = 32;
	TH_intinfo[0].handlers = &TH_ops;
}

int main()
{
	int i;
	H2K_id_t tmpid;
	TH_init_vmblock();
	tmpid.raw = a.id.raw;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	/* Valid */
	for (i = 0; i < 30; i++) {
		tmpid.reserved = i;
		H2K_gp->inthandlers[32+i].param = (void *)tmpid.raw;
	}
	for (i = 0; i < 30; i++) {
		TH_saw_int = 0;
		TH_intno = i;
		H2K_passthru(32+i,NULL,0);
		if (TH_saw_int != 1) FAIL("Didn't get passthrough int");
	}

	/* Invalid CPU */
	tmpid.cpuidx = 1;
	for (i = 0; i < 30; i++) {
		tmpid.reserved = i;
		H2K_gp->inthandlers[32+i].param = (void *)tmpid.raw;
	}
	for (i = 0; i < 30; i++) {
		TH_saw_int = 0;
		TH_intno = -1;
		H2K_passthru(32+i,NULL,0);
		if (TH_saw_int != 0) FAIL("Didn't get passthrough int");
	}

	/* Invalid VM */
	tmpid.cpuidx = 0;
	tmpid.vmidx = 4;
	for (i = 0; i < 30; i++) {
		tmpid.reserved = i;
		tmpid.cpuidx = 0;
		H2K_gp->inthandlers[32+i].param = (void *)tmpid.raw;
	}
	for (i = 0; i < 30; i++) {
		TH_saw_int = 0;
		TH_intno = -1;
		H2K_passthru(32+i,NULL,0);
		if (TH_saw_int != 0) FAIL("Didn't get passthrough int");
	}
	printf("TEST PASSED\n");
	return 0;
}

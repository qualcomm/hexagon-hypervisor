/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stop.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>
#include <thread.h>
#include <hw.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <checker_kernel_locked.h>
#include <globals.h>
#include <vm.h>
#include <asid.h>
#include <create.h>
#include <linear.h>

void H2K_interrupt_restore();

static inline s32_t gpcall_H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me) {
	u32_t gp_sav;
	s32_t ret;
	gp_sav = H2K_get_gp();
	H2K_set_gp(me->gp);
	ret = H2K_thread_create(pc,sp,arg1,prio,vmblock,me);
	H2K_set_gp(gp_sav);
	return ret;
}

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

#define CPUS 6

struct {
	H2K_vmblock_t vm;
	H2K_thread_context contexts[CPUS];
	u32_t pend;
	u32_t en;
	u32_t masks[CPUS];
	u32_t *maskptrs[CPUS];
} TH_vm;

void TH_vm_init()
{
	int i;
	for (i = 0; i < CPUS; i++) {
		memset(&TH_vm.contexts[i],0,sizeof(TH_vm.contexts[i]));
		TH_vm.contexts[i].id.raw = 0;
		TH_vm.contexts[i].id.cpuidx = i;
		TH_vm.contexts[i].id.vmidx = 2;
		TH_vm.contexts[i].vmblock = &TH_vm.vm;
	}
	TH_vm.vm.contexts = &TH_vm.contexts[0];
	TH_vm.vm.max_cpus = CPUS;
	H2K_kg.vmblocks[2] = &TH_vm.vm;
}

u32_t TH_saw_check_sanity = 0;
H2K_thread_context *TH_me = NULL;

u64_t H2K_check_sanity_unlock(u64_t x)
{
	TH_saw_check_sanity++;
	checker_kernel_locked();
	BKL_UNLOCK();
	return x;
}

u64_t H2K_check_sanity(u64_t x)
{
	TH_saw_check_sanity++;
	checker_kernel_locked();
	return x;
}

u64_t H2K_check_sched_mask(u64_t x)
{
	FAIL("Saw sched mask check");
	return x;
}

void test_thread(unsigned int arg)
{
	while (1) /* spin */;
}

unsigned long long int stack;

int main() 
{
	H2K_thread_context *a;
	H2K_thread_context *b;
	H2K_thread_context *c;
	H2K_thread_context *d;
	u32_t asid;
	H2K_vmblock_t *vmblock = &TH_vm.vm;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_runlist_init();
	H2K_readylist_init();
	H2K_lowprio_init();
	H2K_thread_init();
	H2K_asid_table_init();
	TH_vm_init();
	a = &TH_vm.contexts[0];
	b = &TH_vm.contexts[1];
	c = &TH_vm.contexts[2];
	d = &TH_vm.contexts[3];

	asid = H2K_asid_table_inc(0xfeedf00f, H2K_ASID_TRANS_TYPE_LINEAR, H2K_ASID_TLB_INVALIDATE_FALSE, vmblock);

	a->gp = 0x12340000;
	b->gp = c->gp = d->gp = 0x0;
	b->status = c->status = d->status = H2K_STATUS_DEAD;
	vmblock->trapmask = a->trapmask = 0x55ffffff;
	a->ssr_asid = asid;

	vmblock->phys_offset.size = 0;
	vmblock->phys_offset.cccc = 7;
	vmblock->phys_offset.xwru = 0xf;
	vmblock->phys_offset.pages = 0;

	if (H2K_thread_create((u32_t)test_thread,((u32_t)(&stack)),0xdeadbeef,2,vmblock,a)
		!= 0xffffffff) FAIL("Created thread w/o storage");
	vmblock->free_threads = b;
	b->next = c;
	c->next = d;
	d->next = NULL;

	if (H2K_thread_create(((u32_t)test_thread)+1,((u32_t)(&stack)),0xdeadbeef,2,vmblock,a) 
		!= 0xffffffff) FAIL("Created thread w/ misaligned pc");
	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack))+1,0xdeadbeef,2,vmblock,a) 
		!= 0xffffffff) FAIL("Created thread w/ misaligned sp");
	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,902,vmblock,a) 
		!= 0xffffffff) FAIL("Created thread w/ bad prio");

	if (TH_saw_check_sanity != 0) FAIL("Called check_sanity on failure");

	if (gpcall_H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,2,vmblock,a) 
		!= (b->id.raw)) FAIL("Failed to create expected thread");
	if (TH_saw_check_sanity == 0) FAIL("Did not call check_sanity");
	if (H2K_gp->ready[2] != b) FAIL("Thread inserted incorrectly into ready list");
	if (b->prio != 2) FAIL("thread priority set wrong");
	if (b->status == H2K_STATUS_DEAD) FAIL("status field incorrect");
	if (b->r0100 != 0x00000000deadbeefULL) FAIL("Incorrect argument");
	if (b->trapmask != 0x55ffffff) FAIL("Incorrect trap mask");
	if (b->continuation != H2K_interrupt_restore) FAIL("Incorrect continuation");
	if ((b->elr) != ((u32_t)test_thread)) FAIL("Incorrect return address");
	if (b->gp != a->gp) FAIL("Incorrect inheritance of GP");
	if (b->vmblock != vmblock) FAIL("vmblock is non-NULL");

	TH_saw_check_sanity = 0;
	vmblock->pmap = 0x12345678;
	if (H2K_thread_create(((u32_t)test_thread), (u32_t)&stack,0xdeadbeef,2,vmblock,a) 
		!= (c->id.raw)) FAIL("Failed to create expected thread");
	if (c->ssr_asid != a->ssr_asid) FAIL("wrong asid");

#if 0
	vm.max_cpus = 2;
	vm.num_cpus = 2; // initially full
	vm.bestprio = 5;
	vm.trapmask = 0xfa1a1a1a;
	vm.cpu_contexts = vmcontext;
	vm.pmap = 0xb1ab1ab1;
	vm.pmap_type = H2K_ASID_TRANS_TYPE_TABLE;

	/* so we can check if properly decremented */
	asid = H2K_asid_table_inc(vm.pmap, H2K_ASID_TRANS_TYPE_TABLE, H2K_ASID_TLB_INVALIDATE_FALSE, vmblock);

	ret = H2K_thread_create_no_squash(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,6,&vm,&a);
	/* asid count should have gone to 2 and then back to 1 */
	if (H2K_mem_asid_table[asid].fields.count != 1) FAIL("Bad asid count");
	if (ret != -1) FAIL("Exceeded max_cpus");

	vm.num_cpus = 1;
#warning Need to reenable this test and fix when audio becomes unbroken
	//ret = H2K_thread_create_no_squash(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,4,&vm,&a);
	//if (ret != -1) FAIL("Exceeded vm bestprio");

	/* should succeed */
	ret = H2K_thread_create_no_squash(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,6,&vm,&a);
	if (ret == -1) FAIL("Unexpected error");

	if (c->trapmask != 0xfa1a1a1a) FAIL("Bad vm trapmask");
	/* asid should be that of the calling thread, because we're starting an additional vcpu */
	if (c->ssr_asid != a->ssr_asid) FAIL("Bad vm asid 1");
	if (vm.num_cpus != 2) FAIL("Bad vm num_cpus");
	//if (c->vmcpu != 1) FAIL("Bad vmcpu"); /* EJP: now cpuidx field in id */
	if (vmcontext[1] != &c) FAIL("Bad vm context");
	if (c->vmblock != &vm) FAIL("Bad vmblock");

	vm.num_cpus = 0;
	/* should succeed */
	ret = H2K_thread_create_no_squash(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,6,&vm,&a);
	if (ret == -1) FAIL("Unexpected error");

	/* asid should come from vmblock->pmap when num_cpus == 0 */
	if (d.ssr_asid != asid) FAIL("Bad vm asid 2");

	TH_saw_check_sanity = 0;
	if (H2K_thread_create(((u32_t)test_thread),((u32_t)(&stack)),0xdeadbeef,2,0x0,&a) 
		!= (u32_t)(&e)) FAIL("Failed to create expected thread");
	if (TH_saw_check_sanity == 0) FAIL("Did not call check_sanity");

	if (H2K_thread_create((u32_t)test_thread,((u32_t)(&stack)),0xdeadbeef,2,0x0,&a) 
		!= 0xffffffff) FAIL("Created thread w/o storage");
#endif
	puts("TEST PASSED\n");
	return 0;
}


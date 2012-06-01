/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <config.h>
#include <fatal.h>
#include <globals.h>
#include <vm.h>
#include <asid.h>

H2K_kg_t H2K_kg;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

char buf[sizeof(H2K_thread_context)*2] __attribute__((aligned(32)));

#define UNIT sizeof(u32_t)
#define ROUND(expr) ((((expr) + UNIT - 1) / UNIT) * UNIT)

#define DEBUG 1
#ifdef DEBUG
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...) /* NOTHING */
#endif

#define NUM_SIZE_TESTS 8
/* CPUS, INTS, SIZE */
int size_test[NUM_SIZE_TESTS][3] = {
								/* pnd,en,maskptr,mask,phys,ctx,info*/
	{0, 0, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 0+16},
	{1, 0, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 288*1+16},
	{33, 0, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 288*33+16},
	{1, 32, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 8*1+4*1+4*1*1+64+288*1+24},
	{1, 33, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 8*2+4*1+4*2*1+68+288*1+24},
	{32, 32, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 8*1+4*32+4*1*32+64+288*32+24},
	{33, 32, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 8*1+4*33+4*1*33+64+288*33+24},
	{33, 65, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 + 8*3+4*33+4*3*33+132+288*33+24}
};

char vmbuf[65536] __attribute__((aligned(32)));
H2K_vmblock_t *vmblock;
H2K_thread_context a;
s32_t asid;

int TH_expected_intinfo_ints = 0;
void H2K_vm_int_intinfo_init(H2K_vmblock_t *vmblock, u32_t num_ints)
{
	if (num_ints != TH_expected_intinfo_ints) FAIL("intinfo ints");
}

static void __attribute__((noreturn)) foo(u32_t xyzzy)
{
	__builtin_trap();
}

int main()
{
	u32_t i,ret;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_thread_init();
	H2K_fatal_kernel_handler = NULL;
	/* Bad config value */
	ret = H2K_trap_config(CONFIG_MAX,buf,sizeof(buf),0,0,NULL);
	if (ret != 0) FAIL("Bad return value");
	//if (H2K_gp->free_threads) FAIL("trap config failure");
	if (H2K_fatal_kernel_handler != NULL) FAIL("trap config failure");

	/* Configure fatal kernel handler */
	ret = H2K_trap_config(CONFIG_SETFATAL,foo,0,0,0,NULL);
	if (ret != 0) FAIL("Bad return value");
	if (H2K_fatal_kernel_handler != foo) FAIL("Kernel fatal handler error");

	/*** vmblock_size ***/
	for (i = 0; i < NUM_SIZE_TESTS; i++) {
		ret = H2K_trap_config(CONFIG_VMBLOCK_SIZE, NULL, size_test[i][0], size_test[i][1], 0, NULL);
		DPRINTF("\n\n%d cpus, %d ints, expect %d:\n", size_test[i][0], size_test[i][1], size_test[i][2]);
		DPRINTF("\nvmblock size %d, aligned size %d, total size %d\n",
				 sizeof(H2K_vmblock_t), ROUND(sizeof(H2K_vmblock_t)), ret);
		if (ret != size_test[i][2]) FAIL("Wrong size");
	}

	/*** vmblock_init ***/
	/* SET_STORAGE aligned pointer*/
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmbuf, SET_STORAGE, 0, 0, NULL);
	DPRINTF("\nvmbuf %08x, ret %08x\n", (u32_t)vmbuf, ret);
	if (ret == 0) FAIL("Unexpected error 1");
	if ((char *)ret != vmbuf) FAIL("Unnecessary alignment");

	/* SET_STORAGE unaligned pointer */
	vmblock = (H2K_vmblock_t *)H2K_trap_config(CONFIG_VMBLOCK_INIT, vmbuf + 1, SET_STORAGE, 3, 0, NULL);
	DPRINTF("\nvmbuf %08x, vmblock %08x\n", (u32_t)vmbuf, (u32_t)vmblock);
	if (vmblock == 0) FAIL("Unexpected error 2");
	if ((u32_t)vmblock != ROUND((u32_t)vmblock)) FAIL("Not aligned");

	/* SET_PMAP_TYPE bad type*/
	DPRINTF("SET_PMAP_TYPE\n\n");
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_PMAP_TYPE, 0, H2K_ASID_TRANS_TYPE_XXX_LAST, NULL);
	if (ret != 0) FAIL("Missed bad translation type");

	/* SET_PMAP_TYPE */
	H2K_asid_table_init();
	asid = H2K_asid_table_inc(0xfeedf00f, H2K_ASID_TRANS_TYPE_TABLE, H2K_ASID_TLB_INVALIDATE_FALSE);
	if (asid < 0) FAIL("H2K_asid_table_inc");
	a.ssr_asid = asid;
	DPRINTF("ASID %d  ptb %08x\n\n", a.ssr_asid, H2K_mem_asid_table[a.ssr_asid].ptb);
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_PMAP_TYPE, 0, 0, &a);
	if (ret == 0) FAIL("Unexpected error 3");
	if (ret != (u32_t)vmblock) FAIL("vmblock pointer changed");
	DPRINTF("pmap %08x  type %d\n", vmblock->pmap, vmblock->pmap_type);
	if (vmblock->pmap != 0xfeedf00f) FAIL("Wrong ptb");
	if (vmblock->pmap_type != H2K_mem_asid_table[a.ssr_asid].transtype) FAIL("Wrong pmap type");

	DPRINTF("SET_PRIO_TRAPMASK\n\n");
	/* SET_PRIO_TRAPMASK bad prio */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_PRIO_TRAPMASK, MAX_PRIO + 1, 0, NULL);
	if (ret != 0) FAIL("Missed bad prio");

	/* SET_PRIO_TRAPMASK */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_PRIO_TRAPMASK, 11, 0xfeeefaaa, NULL);
	if (ret == 0) FAIL("Unexpected error 4");
	if (ret != (u32_t)vmblock) FAIL("vmblock pointer changed");
	if (vmblock->bestprio != 11) FAIL("Bad prio");
	if (vmblock->trapmask != 0xfeeefaaa) FAIL("Bad trapmask");

	/* SET_CPUS_INTS bad cpus */
	DPRINTF("SET_CPUS_INTS\n\n");
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_CPUS_INTS, MAX_VM_CPUS + 1, 0, NULL);
	if (ret != 0) FAIL("Missed bad cpus");

		/* SET_CPUS_INTS bad ints */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_CPUS_INTS, 1, MAX_VM_INTS + 1, NULL);
	if (ret != 0) FAIL("Missed bad ints");

	/* SET_CPUS_INTS */
	TH_expected_intinfo_ints = 65;
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_CPUS_INTS, 33, 65, NULL);
	if (ret == 0) FAIL("Unexpected error 5");
	if (ret != (u32_t)vmblock) FAIL("vmblock pointer changed");

	if (vmblock->max_cpus != 33) FAIL("Bad max_cpus");
	if (vmblock->num_cpus != 0) FAIL("Bad num_cpus");
	if (vmblock->num_ints != 65) FAIL("Bad num_ints");

	if (vmblock->contexts != (H2K_thread_context *)((char *)vmblock + sizeof(H2K_vmblock_t))) FAIL("Bad cpu_contexts base");
	if (vmblock->intinfo != (H2K_vm_int_opinfo_t *)((char *)(vmblock->contexts) + 33*288)) FAIL("Bad intinfo base");
	if (vmblock->percpu_mask !=  (bitmask_t **)((char *)(vmblock->intinfo) + 3*8)) FAIL ("Bad percpu_mask base");

	for (i = 0; i < 33; i++) {
		DPRINTF("i %d  ptr %08x  expect %08x\n", i, (u32_t)vmblock->percpu_mask[i], (u32_t)((char *)(vmblock->percpu_mask) + i*12));
		if (vmblock->percpu_mask[i] !=  (bitmask_t *)((char *)(vmblock->percpu_mask) + 33*4 + i*12)) FAIL("Bad percpu_mask pointer");
	}

	if (vmblock->pending != (bitmask_t *)((char *)(vmblock->percpu_mask) + 33*4 + 33*12)) FAIL("Bad pending base");
	if (vmblock->enable !=  (bitmask_t *)((char *)(vmblock->pending) + 12)) FAIL("Bad enable base");

	if (vmblock->int_v2p != (physint_t *)((char *)(vmblock->enable) + 12)) FAIL("Bad int_v2p base");
	
	DPRINTF("MAP_PHYS_INTR\n\n");
	/* MAP_PHYS_INTR bad vint*/
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, MAP_PHYS_INTR, vmblock->num_ints, 0, NULL);
	if (ret != 0) FAIL("Missed vint # too big");

	/* MAP_PHYS_INTR bad pint */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, MAP_PHYS_INTR, 0, MAX_INTERRUPTS + 1, NULL);
	if (ret != 0) FAIL("Missed pint # too big");

	/* MAP_PHYS_INTR */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, MAP_PHYS_INTR, 32, 13, NULL);
	if (ret == 0) FAIL("Unexpected error 7");
	if (ret != (u32_t)vmblock) FAIL("vmblock pointer changed");

	if (vmblock->int_v2p[32] != 13) FAIL("Bad int_v2p");

	/* 0 interrupts */
	DPRINTF("0 interrupts\n\n");
	TH_expected_intinfo_ints = 0;
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vmblock, SET_CPUS_INTS, 33, 0, NULL);
	if (ret == 0) FAIL("Unexpected error 6");
	if (ret != (u32_t)vmblock) FAIL("vmblock pointer changed");

	if (vmblock->max_cpus != 33) FAIL("Bad max_cpus 0");
	if (vmblock->num_cpus != 0) FAIL("Bad num_cpus 0");
	if (vmblock->num_ints != 0) FAIL("Bad num_ints 0");
	if (vmblock->percpu_mask != NULL) FAIL("percpu_mask non-NULL");
	if (vmblock->pending != NULL) FAIL("pending non-NULL");
	if (vmblock->enable != NULL) FAIL("enable non-NULL");
	if (vmblock->int_v2p != NULL) FAIL("int_v2p non-NULL");

	

	puts("TEST PASSED\n");
	return 0;
}


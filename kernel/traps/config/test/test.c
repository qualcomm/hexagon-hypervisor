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
#include <alloc.h>
#include <trace.h>

H2K_kg_t H2K_kg;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

#define UNIT sizeof(u32_t)
#define ROUND(expr) ((((expr) + UNIT - 1) / UNIT) * UNIT)

#define DEBUG 1
#ifdef DEBUG
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...) /* NOTHING */
#endif

#define NUM_SIZE_TESTS 8

#define OK_CPUS 33
#define OK_INTS 65

/* CPUS, INTS, SIZE */
int size_test[NUM_SIZE_TESTS][3] = {
								                                                /* pnd,  en,    maskptr, mask,    phys, ctx,     info*/
	{0, 0,   ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  0   + 0    + 0      + 0      + 64  + 0      + 16},
	{1, 0,   ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  0   + 0    + 0      + 0      + 64  + 288*1  + 16},
	{33, 0,  ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  0   + 0    + 0      + 0      + 64  + 288*33 + 16},
	{1, 32,  ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  4*1 + 4*1  + 4*1    + 4*1*1  + 128 + 288*1  + 24},
	{1, 33,  ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  4*2 + 4*2  + 4*1    + 4*2*1  + 132 + 288*1  + 24},
	{32, 32, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  4*1 + 4*1  + 4*32   + 4*1*32 + 128 + 288*32 + 24},
	{33, 32, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  4*1 + 4*1  + 4*33   + 4*1*33 + 128 + 288*33 + 24},
	{33, 65, ROUND(sizeof(H2K_vmblock_t)) + H2K_VMBLOCK_ALIGN - 1 +  4*3 + 4*3  + 4*33   + 4*3*33 + 196 + 288*33 + 24}
};

H2K_mem_alloc_tag_t Heap[DEFAULT_ALLOC_HEAP_SIZE] __attribute__((aligned(ALLOC_UNIT))) = {{{.size = 0, .free = 0}}};

H2K_vmblock_t *vmblock;
H2K_thread_context a;
s32_t asid;

int TH_expected_intinfo_ints = 0;
void H2K_vm_int_intinfo_init(H2K_vmblock_t *vmblock, u32_t num_ints)
{
	if (num_ints != TH_expected_intinfo_ints) FAIL("intinfo ints");
}

int main()
{
	u32_t i, vm, ret;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_thread_init();
	H2K_trace_init();
	H2K_mem_alloc_init(Heap, DEFAULT_ALLOC_HEAP_SIZE);
	/* Bad config value */
	vm = H2K_trap_config(CONFIG_MAX,0,0,0,0,NULL);
	if (vm != 0) FAIL("Bad return value");

	/* SET_CPUS_INTS */
	/* SET_CPUS_INTS bad cpus */
	DPRINTF("SET_CPUS_INTS\n\n");
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_CPUS_INTS, H2K_ID_MAX_CPUS + 1, 0, NULL);
	if (ret!= 0) FAIL("Missed bad cpus");

		/* SET_CPUS_INTS bad ints */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_CPUS_INTS, 1, MAX_VM_INTS + 1, NULL);
	if (ret!= 0) FAIL("Missed bad ints");

	TH_expected_intinfo_ints = OK_INTS;
	vm = H2K_trap_config(CONFIG_VMBLOCK_INIT, 0, SET_CPUS_INTS, OK_CPUS, OK_INTS, NULL);
	if (vm == 0) FAIL("Unexpected error 5");

	vmblock = H2K_gp->vmblocks[vm];

	if (vmblock->max_cpus != OK_CPUS) FAIL("Bad max_cpus");
	if (vmblock->num_cpus != 0) FAIL("Bad num_cpus");
	if (vmblock->num_ints != OK_INTS) FAIL("Bad num_ints");

	if (vmblock->contexts != (H2K_thread_context *)(void *)((char *)vmblock + sizeof(H2K_vmblock_t))) FAIL("Bad cpu_contexts base");
	if (vmblock->intinfo != (H2K_vm_int_opinfo_t *)(void *)((char *)(vmblock->contexts) + OK_CPUS*288)) FAIL("Bad intinfo base");
	if (vmblock->percpu_mask !=  (bitmask_t **)(void *)((char *)(vmblock->intinfo) + 3*8)) FAIL ("Bad percpu_mask base");

	for (i = 0; i < OK_CPUS; i++) {
		DPRINTF("i %d  ptr %08x  expect %08x\n", i, (u32_t)vmblock->percpu_mask[i], (u32_t)((char *)(vmblock->percpu_mask) + OK_CPUS*4 + i*12));
		if (vmblock->percpu_mask[i] !=  (bitmask_t *)(void *)((char *)(vmblock->percpu_mask) + OK_CPUS*4 + i*12)) FAIL("Bad percpu_mask pointer");
	}

	if (vmblock->pending != (bitmask_t *)(void *)((char *)(vmblock->percpu_mask) + OK_CPUS*4 + OK_CPUS*12)) FAIL("Bad pending base");
	if (vmblock->enable !=  (bitmask_t *)(void *)((char *)(vmblock->pending) + 12)) FAIL("Bad enable base");

	if (vmblock->int_v2p != (physint_t *)(void *)((char *)(vmblock->enable) + 12)) FAIL("Bad int_v2p base");
	
	/* SET_PMAP_TYPE bad type*/
	DPRINTF("SET_PMAP_TYPE\n\n");
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PMAP_TYPE, 0, H2K_ASID_TRANS_TYPE_XXX_LAST, NULL);
	if (ret!= 0) FAIL("Missed bad translation type");

	/* SET_PMAP_TYPE */
	H2K_asid_table_init();
	asid = H2K_asid_table_inc(0xfeedf00f, H2K_ASID_TRANS_TYPE_TABLE, H2K_ASID_TLB_INVALIDATE_FALSE, 0, vmblock);
	if (asid < 0) FAIL("H2K_asid_table_inc");
	a.ssr_asid = asid;
	DPRINTF("ASID %d  ptb %08x\n\n", a.ssr_asid, H2K_gp->asid_table[a.ssr_asid].ptb);
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PMAP_TYPE, 0, 0, &a);
	if (ret == 0) FAIL("Unexpected error 3");

	DPRINTF("pmap %08x  type %d\n", vmblock->guestmap.ptb, vmblock->guestmap.fields.type);
	if (vmblock->guestmap.ptb != 0xfeedf00f) FAIL("Wrong ptb");
	if (vmblock->guestmap.fields.type != H2K_gp->asid_table[a.ssr_asid].fields.type) FAIL("Wrong pmap type");

	DPRINTF("SET_PRIO_TRAPMASK\n\n");
	/* SET_PRIO_TRAPMASK bad prio */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PRIO_TRAPMASK, MAX_PRIO + 1, 0, NULL);
	if (ret!= 0) FAIL("Missed bad prio");

	/* SET_PRIO_TRAPMASK */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, SET_PRIO_TRAPMASK, 11, 0xfeeefaaa, NULL);
	if (ret == 0) FAIL("Unexpected error 4");

	if (vmblock->bestprio != 11) FAIL("Bad prio");
	if (vmblock->trapmask != 0xfeeefaaa) FAIL("Bad trapmask");

	DPRINTF("MAP_PHYS_INTR\n\n");
	/* MAP_PHYS_INTR bad vint*/
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, MAP_PHYS_INTR, vmblock->num_ints + PERCPU_INTERRUPTS, CONFIG_PHYSINT_CPUID(vmblock->num_ints + PERCPU_INTERRUPTS - 1, OK_CPUS - 1), NULL);
	if (ret!= 0) FAIL("Missed vint # too big");

	/* MAP_PHYS_INTR bad pint */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, MAP_PHYS_INTR, OK_INTS - 1, MAX_INTERRUPTS + 1, NULL);
	if (ret!= 0) FAIL("Missed pint # too big");

	/* MAP_PHYS_INTR */
	ret = H2K_trap_config(CONFIG_VMBLOCK_INIT, vm, MAP_PHYS_INTR, OK_INTS - 1, CONFIG_PHYSINT_CPUID(13, 0), NULL);
	if (ret == 0) FAIL("Unexpected error 6");

	if (vmblock->int_v2p[OK_INTS - 1] != 13) FAIL("Bad int_v2p");

	/* 0 interrupts */
	DPRINTF("0 interrupts\n\n");
	TH_expected_intinfo_ints = 0;
	vm = H2K_trap_config(CONFIG_VMBLOCK_INIT, 0, SET_CPUS_INTS, OK_CPUS, 0, NULL);
	if (vm == 0) FAIL("Unexpected error 7");

	vmblock = H2K_gp->vmblocks[vm];

	if (vmblock->max_cpus != OK_CPUS) FAIL("Bad max_cpus 0");
	if (vmblock->num_cpus != 0) FAIL("Bad num_cpus 0");
	if (vmblock->num_ints != 0) FAIL("Bad num_ints 0");
	if (vmblock->percpu_mask != NULL) FAIL("percpu_mask non-NULL");
	if (vmblock->pending != NULL) FAIL("pending non-NULL");
	if (vmblock->enable != NULL) FAIL("enable non-NULL");

	

	puts("TEST PASSED\n");
	return 0;
}


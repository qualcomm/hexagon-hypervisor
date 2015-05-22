/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <linear.h>
#include <h2_common_pmap.h>
#include <cpuint.h>
#include <shint.h>
#include <max.h>

#include <h2.h>
#include <h2_vm.h>
#include <h2_config.h>
#include <h2_vmtraps.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_VCPUS 400
#define SHARED_INTS 0

#define CHILD_INTERRUPT 14
#define VM_STATUS_REBOOT 3

#ifdef NO_PRINT
#define PRINTF(format, args...)
#else
#define PRINTF(format, args...) printf (format , ##args)
#endif

void FAIL(const char *str)
{
	PRINTF("t32boot: FAIL %s\n", str);
	exit(1);
}

unsigned long vm_setup(unsigned int num_cpus, short num_ints, u32_t trans, unsigned long trapmask, translation_type pt) {

	unsigned long vm, ret;
	H2K_offset_t base;
	base.size = 6;
	base.cccc = 7;
	base.xwru = 0xf;
	base.pages = 0;

	vm = h2_config_vmblock_init(0, SET_CPUS_INTS, num_cpus, num_ints);
	if (vm == 0) FAIL("SET_CPUS_INTS");

	ret = h2_config_vmblock_init(vm, SET_PMAP_TYPE, base.raw, pt);
	if (ret != vm) {
		PRINTF("ret %08x\n", (unsigned int)ret);
		FAIL("SET_PMAP_TYPE");
	}

	if (pt == H2K_ASID_TRANS_TYPE_OFFSET) {
		/* Memory map:  0 ... Linux ... frame buffer ... H2 ... boot VM ... ucos */
		//		ret = h2_config_vmblock_init(vm, SET_FENCES, 0x0, FRAME_BUFFER);
		ret = h2_config_vmblock_init(vm, SET_FENCES, 0x00000000, 0xff000000);
		if (ret != vm) {
			PRINTF("set fences ret %08x\n", (unsigned int)ret);
			FAIL("SET_FENCES");
		} else {
			PRINTF("set fences ok\n");
		}
	}

	if (h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0, trapmask) != vm) {
		FAIL("SET_PRIO_TRAPMASK");
	}

	return vm;
}

volatile unsigned int *t32_vm_entry_p = (volatile unsigned int *)0x8D4FFFFC;

unsigned int vm_best_prio = 0;

unsigned long boot_vm() {
	unsigned int newvm = vm_setup(NUM_VCPUS, 
		SHARED_INTS, 
		0, 
		0xFFFFFFFF,
		H2K_ASID_TRANS_TYPE_OFFSET);
	PRINTF("vm set up entry=%x\n",*t32_vm_entry_p);
	if (h2_vmboot((void *)(*t32_vm_entry_p), (void *)0x20000000, *t32_vm_entry_p, vm_best_prio, newvm) == -1) FAIL("vmboot");
	return newvm;
}

extern void bootvm_vectors();

void handle_child_int() {
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, CHILD_INTERRUPT, 0);
}

int main()
{
	unsigned int new_vm;
	h2_sem_t sem;
	h2_sem_init_val(&sem,0);
	PRINTF("autoboot: H2 started\n");
	// EJP: 5/19/2015: L2 cache size reconfiguration is buggy, we lose data
	// h2_hwconfig_l2cache_size(3,1);//1==64KB,2==128KB,3==256KB,4==512KB
#if 0
	// EJP: testing
	extern unsigned int t32_test_load();
	t32_vm_entry = t32_test_load();
	t32_vm_loaded_flag = 1;
#endif
	new_vm = boot_vm();
	PRINTF("started vm %x\n",new_vm);
	while (1) h2_sem_down(&sem);
	return 0; // make gcc happy
}

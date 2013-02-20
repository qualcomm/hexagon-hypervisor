/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <linear.h>
#include <bootmap_macros.h>
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

#define LINUX_NUM_VCPU 3
#define UCOS_NUM_VCPU 1
#define TOTAL_INTS 128
#define SHARED_INTS (TOTAL_INTS - PERCPU_INTERRUPTS)
#define VCPU_STACK_SIZE 1024
#define LINUX_VM_PRIO 3
#define UCOS_VM_PRIO 1

#define HW_TIMER_INT 10
#define LINUX_TIMER_INT 2

#define VM_STATUS_REBOOT 3
#define CHILD_INTERRUPT 14

H2K_offset_t linux_offset = {{
	.size = SIZE_4M,
	.cccc = L1WB_L2C,
	.xwru = URWX,
	.pages = (LINUX_OFFSET_ADDR >> PAGE_BITS)
	}};

unsigned long long int linux_vcpu_stacks[LINUX_NUM_VCPU][VCPU_STACK_SIZE];
unsigned long long int ucos_vcpu_stacks[UCOS_NUM_VCPU][VCPU_STACK_SIZE];

#ifdef NO_PRINT
#define PRINTF(format, args...)
#else
#define PRINTF(format, args...) printf (format , ##args)
#endif

#ifdef UCOS

/* can't use offset because ucos needs to write to frame-buffer addresses */
#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) MEMORY_MAP_THREAD(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN)

H2K_linear_fmt_t ucos_pmap[] = {
#include "../ucos/pmap_ucos.def"
	{ .raw = 0 },
};

extern void ucos_image_start();
extern void ucos_image_end();
extern void ucos_loadaddr();
extern void ucos_entry();
#endif

extern void linux_stext();

void FAIL(const char *str)
{
	PRINTF("linux: FAIL %s\n", str);
	exit(1);
}

void fatal () {
	PRINTF("H2 kernel: Oops\n");
	exit (1);
}

unsigned long vm_setup(char num_cpus, short num_ints, u32_t trans, unsigned long trapmask, translation_type pt) {

	unsigned long vm, ret;

	vm = h2_config_vmblock_init(0, SET_CPUS_INTS, num_cpus, num_ints);
	if (vm == 0) FAIL("SET_CPUS_INTS");

	ret = h2_config_vmblock_init(vm, SET_PMAP_TYPE, trans, pt);
	if (ret != vm) {
		PRINTF("ret %08x\n", (unsigned int)ret);
		FAIL("SET_PMAP_TYPE");
	}

	if (pt == H2K_ASID_TRANS_TYPE_OFFSET) {
		ret = h2_config_vmblock_init(vm, SET_FENCES, (unsigned int)linux_stext, H2K_GUEST_END);
		if (ret != vm) {
			PRINTF("ret %08x\n", (unsigned int)ret);
			FAIL("SET_FENCES");
		}
	}

	if (h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0, trapmask) != vm) {
		FAIL("SET_PRIO_TRAPMASK");
	}

	return vm;
}

void setup_ints(unsigned long vm, char num_cpus) {

	int i;

	for (i = 0; i < TOTAL_INTS; i++) {
		if (h2_config_vmblock_init(vm, MAP_PHYS_INTR, i, H2_CONFIG_PHYSINT_CPUID(i, num_cpus - 1)) != vm) {
				FAIL("MAP_PHYS_INTR");
		}
	}
}

void boot_ucos() {

#ifdef UCOS
	unsigned long ucos_vm;

	PRINTF("ucos: start boot\n");
	ucos_vm = vm_setup(UCOS_NUM_VCPU, SHARED_INTS, (u32_t)ucos_pmap, 0xffffffff, H2K_ASID_TRANS_TYPE_LINEAR);
	PRINTF("ucos: vm set up\n");

	PRINTF("ucos: loading to 0x%08x from 0x%08x, size 0x%08x\n", (unsigned int)ucos_loadaddr, (unsigned int)ucos_image_start, (unsigned int)(ucos_image_end - ucos_image_start));
	memcpy(ucos_loadaddr, ucos_image_start, ucos_image_end - ucos_image_start);

	PRINTF("ucos: loaded\n");

	if (h2_vmboot(ucos_entry, &ucos_vcpu_stacks[0][VCPU_STACK_SIZE - 1],
								0, UCOS_VM_PRIO, ucos_vm) == -1) FAIL("ucos vmboot");

	PRINTF ("ucos: booted\n");
#endif
}

unsigned long boot_linux(char fname[]) {

	unsigned long linux_vm = 0;

#ifdef LINUX
	PRINTF("linux: start boot\n");

	linux_vm = vm_setup(LINUX_NUM_VCPU, SHARED_INTS, linux_offset.raw, 0x1, H2K_ASID_TRANS_TYPE_OFFSET);
	setup_ints(linux_vm, LINUX_NUM_VCPU);
	PRINTF("linux: vm set up\n");

#ifndef NO_LOAD
	size_t count;
	unsigned long *ptr = (unsigned long *)LINUX_LOAD_ADDR;
	FILE *file;

	file = fopen(fname, "r");
	if (file == NULL) FAIL("fopen");

	PRINTF("linux: loading %s to 0x%08x\n", fname, (unsigned int)ptr);
	do {
		count = fread(ptr, sizeof(unsigned long), 0x40000, file);
		ptr += count;
		if (count < 0x40000 && ferror(file)) FAIL("ferror");
	} while (!feof(file));
	fclose(file);
	PRINTF ("linux: loaded %s\n", fname);
#endif

	/* FIXME: set up I$ prefetch and DMT for linux */
	unsigned long val;
#if ARCHV >= 4
	__asm__ __volatile__ 
		(
		 /* " %0 = usr\n" */
		 /* " %0 = setbit(%0, #16)\n"  // I$ prefetch */  // Linux needs to set this
		 /* " usr = %0\n" */
		 " %0 = syscfg\n"
		 " %0 = setbit(%0, #15)\n"  // DMT
		 " %0 = clrbit(%0, #13)\n"  // BQ
		 " syscfg = %0\n"
		 : "=&r" (val)
		 );
#else
	__asm__ __volatile__ 
		(
		 " %0 = ssr\n"
		 " %0 = setbit(%0, #22)\n"  // I$ prefetch
		 " ssr = %0\n"
		 : "=&r" (val)
		 );
#endif

	if (h2_vmboot(linux_stext, &linux_vcpu_stacks[0][VCPU_STACK_SIZE - 1],
								0, LINUX_VM_PRIO, linux_vm) == -1) FAIL("linux vmboot");

	PRINTF ("linux: booted\n");
#endif
	return linux_vm;
}

extern void bootvm_vectors();

int main(int argc, char *argv[]) {

	char fname[256] = "vmlinux.bin";
	unsigned long linux_vm;
	int status, cpus;

	if (argc > 1) {
		sscanf(argv[1], "%s", fname);
	}

	h2_config_setfatal(fatal);
	PRINTF("loadlinux: H2 started\n");

	h2_vmtrap_setvec(bootvm_vectors);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, CHILD_INTERRUPT, 0);
	h2_vmtrap_intop(H2K_INTOP_LOCEN, CHILD_INTERRUPT, 0);

	boot_ucos();

	do {
		linux_vm = boot_linux(fname);

		do {  // wait for all child VM cpus to vmstop
			h2_vmtrap_wait();
			status = h2_vmstatus(VMOP_STATUS_STATUS, linux_vm);
			printf("linux VM %lu status %d\n", linux_vm, status);
			cpus = h2_vmstatus(VMOP_STATUS_CPUS, linux_vm);
			printf("VM %lu Live CPUs: %d\n", linux_vm, cpus);
		} while (cpus != 0);

		h2_vmfree(linux_vm);
	} while (status == VM_STATUS_REBOOT);

	//h2_thread_stop(0);
	return 0; // make gcc happy
}

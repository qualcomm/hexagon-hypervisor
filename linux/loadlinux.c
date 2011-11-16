/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <linear.h>
#include <bootmap_macros.h>
#include <vmint.h>
#include <max.h>

#include <h2.h>
#include <h2_vm.h>
#include <h2_config.h>
#include <h2_fastint.h>
#include <h2_vmtraps.h>

#include <stdio.h>
#include <stdlib.h>

#define NUM_VCPU 6
#define HW_INTS 32
#define INTS_PER_VCPU 32
#define CONTEXT_SIZE 256
#define VCPU_STACK_SIZE 1024
#define VM_PRIO 3

#define HW_TIMER_INT 2
#define LINUX_TIMER_INT 2

char vcpu_contexts[(NUM_VCPU) * CONTEXT_SIZE] __attribute__((aligned(32)));
char vmblock_space[65536];
void *vmb;

unsigned long long int vcpu_stacks[NUM_VCPU][VCPU_STACK_SIZE];

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) MEMORY_MAP_THREAD(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN)

H2K_linear_fmt_t pmap[] = {
#include "pmap.def"
	{ .raw = 0 },
};

struct timerif {
	unsigned int match;
	unsigned int count;
	unsigned int enable;
	unsigned int clear;
};

volatile struct timerif *timerptr = (void *)0xAB000000;

void FAIL(const char *str)
{
	puts("linux: FAIL");
	puts(str);
	exit(1);
}

void fatal () {
	printf("H2 kernel: Oops\n");
	exit (1);
}

int fastint(int intno) {

	/* remap timer interrupt */
	//	if (intno == HW_TIMER_INT) intno = LINUX_TIMER_INT;

	H2K_thread_context *fic;

#if __QDSP6_ARCH__ >= 4
		__asm__ __volatile__ 
			(
			 " %0 = sgp0\n"
			 : "=r" (fic)
			 );
#else
		__asm__ __volatile__ 
			(
			 " %0 = sgp\n"
			 : "=r" (fic)
			 );
#endif

		fic->vmblock = vmb;
		fic->vmcpu = 0;
		h2_vmtrap_intop(H2K_INTOP_POST, intno, 0);

	//	H2K_vm_interrupt_post(vmb, 0, intno);
	return 1; // re-enable
}

void *vm_setup() {

	unsigned long vmb_size;
	void *ret;

	int i;

	h2_init(0);
	h2_config_setfatal(fatal);
	h2_config_add_thread_storage(vcpu_contexts, sizeof(vcpu_contexts));
	printf("linux: H2 started\n");

	vmb_size = h2_config_vmblock_size(NUM_VCPU, INTS_PER_VCPU);
	printf("linux: vmb size %d\n", (int)vmb_size);

	vmb =	h2_config_vmblock_init(vmblock_space, SET_STORAGE_IDENT, 0, 0);
	if (vmb == NULL) FAIL("SET_STORAGE_IDENT");
	printf("linux: vmb %08x\n", (unsigned int)vmb);

	ret = h2_config_vmblock_init(vmb, SET_PMAP_TYPE, (unsigned int)pmap, H2K_ASID_TRANS_TYPE_LINEAR);
	if (ret != vmb) {
		printf("linux: ret %08x\n", (unsigned int)ret);
		FAIL("SET_PMAP_TYPE");
	}

	if (h2_config_vmblock_init(vmb, SET_PRIO_TRAPMASK, 0, 0x1) != vmb) { // angel trap enabled
		FAIL("SET_PRIO_TRAPMASK");
	}

	if (h2_config_vmblock_init(vmb, SET_CPUS_INTS, NUM_VCPU, INTS_PER_VCPU) != vmb) {
		FAIL("SET_CPUS_INTS");
	}

	for (i = 0; i < HW_INTS; i++) {
		/* if (i == LINUX_TIMER_INT) { */
		/* 	if (h2_config_vmblock_init(vmb, MAP_PHYS_INTR, LINUX_TIMER_INT, HW_TIMER_INT) != vmb) { */
		/* 		FAIL("MAP_PHYS_INTR"); */
		/* 	} */
		/* } else { */
		/* if (i != VM_IPI_INT && i != HW_TIMER_INT */
		/* 		&& h2_config_vmblock_init(vmb, MAP_PHYS_INTR, i, i) != vmb) { */
		/* 	FAIL("MAP_PHYS_INTR"); */
		/* } */
		/* } */

		if (i != RESCHED_INT && i != VM_IPI_INT) {
			if (h2_config_vmblock_init(vmb, MAP_PHYS_INTR, i, i) != vmb) {
				FAIL("MAP_PHYS_INTR");
			}

			h2_register_fastint(i, fastint);
		}
	}

	/* FIXME: Linux should do this */
	for (i = 0; i < INTS_PER_VCPU; i++) {
		H2K_vm_interrupt_enable(vmb, i);
	}

	/* timerptr->clear = 1; */
	/* timerptr->match = 32; */
	/* timerptr->enable = 3; */
	/* puts("linux: Timer set up..."); */

	return vmb;
}

extern void linux_stext();

int main(int argc, char *argv[]) {

	void *vmb;

	size_t count;
	unsigned long *ptr = 0x0;
	FILE *file;
	char fname[256] = "vmlinux.bin";

	puts("linux: start boot");

	vmb = vm_setup();
	puts("linux: vm set up");

	if (argc > 1) {
		sscanf(argv[1], "%s", fname);
	}

	file = fopen(fname, "r");
	if (file == NULL) FAIL("fopen");

	printf("linux: loading %s\n", fname);
	do {
		count = fread(ptr, sizeof(unsigned long), 0x40000, file);
		ptr += count;
		if (count < 0x40000 && ferror(file)) FAIL("ferror");
	} while (!feof(file));
	printf ("linux: loaded %s\n", fname);

	/* go to guest mode */
/* 	asm volatile  */
/* 		( */
/* 		 " r0 = ssr \n" */
/* 		 " r0 = setbit(r0,#17) \n"  // ex */
/* 		 " r0 = setbit(r0,#16) \n"  // user */
/* #if ARCHV >= 4 */
/* 		 " r0 = setbit(r0,#19) \n"  // guest */
/* #else */
/* 		 " r0 = setbit(r0,#13) \n"  // guest */
/* #endif */
/* 		 " ssr = r0 \n" */
/* 		 " r0.h = #hi(1f) \n" */
/* 		 " r0.l = #lo(1f) \n" */
/* 		 " elr = r0 \n" */
/* 		 " rte \n" */
/* 		 "1: nop \n" */
/* 		 : : : "r0" */
/* 		 ); */

	if (h2_vmboot(linux_stext, &vcpu_stacks[0][VCPU_STACK_SIZE - 1],
								0, VM_PRIO, vmb) == -1) FAIL("vmboot");

	h2_thread_stop();
	return 0; // make gcc happy
}

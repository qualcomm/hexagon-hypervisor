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
#include <globals.h>

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

H2K_offset_t linux_offset = {{
	.size = SIZE_4M,
	.cccc = L1WB_L2C,
	.xwru = URWX,
	.pages = (LINUX_OFFSET_ADDR >> PAGE_BITS)
	}};

char linux_vmblock_space[65536];
char ucos_vmblock_space[65536];
void *linux_vmb;

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

void FAIL(const char *str)
{
	PRINTF("linux: FAIL %s\n", str);
	exit(1);
}

void fatal () {
	PRINTF("H2 kernel: Oops\n");
	exit (1);
}

void *vm_setup(char num_cpus, short num_ints, char vmblock_space[], u32_t trans, unsigned long trapmask, translation_type pt) {

	unsigned long vmb_size;
	void *ret;
	void *vmb;

	vmb_size = h2_config_vmblock_size(num_cpus, num_ints);
	PRINTF("vmb size %d\n", (int)vmb_size);

	vmb =	h2_config_vmblock_init(vmblock_space, SET_STORAGE, 0, 0);
	if (vmb == NULL) FAIL("SET_STORAGE");
	PRINTF("vmb %08x\n", (unsigned int)vmb);

	ret = h2_config_vmblock_init(vmb, SET_PMAP_TYPE, trans, pt);
	if (ret != vmb) {
		PRINTF("ret %08x\n", (unsigned int)ret);
		FAIL("SET_PMAP_TYPE");
	}

	if (pt == H2K_ASID_TRANS_TYPE_OFFSET) {
		ret = h2_config_vmblock_init(vmb, SET_FENCES, 0x0, 0xff000000);
		if (ret != vmb) {
			PRINTF("ret %08x\n", (unsigned int)ret);
			FAIL("SET_FENCES");
		}
	}

	if (h2_config_vmblock_init(vmb, SET_PRIO_TRAPMASK, 0, trapmask) != vmb) {
		FAIL("SET_PRIO_TRAPMASK");
	}

	if (h2_config_vmblock_init(vmb, SET_CPUS_INTS, num_cpus, num_ints) != vmb) {
		FAIL("SET_CPUS_INTS");
	}
	return vmb;
}

void setup_ints(void *vmb, char num_cpus) {

	int i, j;

	for (i = 0; i < TOTAL_INTS; i++) {
		if (i != RESCHED_INT
#ifdef H2K_L2_CONTROL
				&& i != L2_CORE_INTERRUPT
#endif
				&& i != VM_IPI_INT
				&& i != TIMER_INT) {
			/* Send per-cpu HW interrupts to the first configured cpu, which is
				 num_cpus - 1, in case Linux doesn't boot all its configured cpus */
			if (h2_config_vmblock_init(vmb, MAP_PHYS_INTR, i, H2_CONFIG_PHYSINT_CPUID(i, num_cpus - 1)) != vmb) {
				FAIL("MAP_PHYS_INTR");
			}
			//h2_register_fastint(i, fastint);
		}
	}

	/* FIXME: Linux should do the per-cpu and global enables */
	/* can't call the trap here since it would use the boot vmblock */
	for (i = 0; i < PERCPU_INTERRUPTS; i++) {
		for (j = 0; j < ((H2K_vmblock_t *)vmb)->max_cpus; j++) {

			__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
			H2K_vm_cpuint_enable(vmb, &(((H2K_vmblock_t *)vmb)->contexts[j]), i, ((H2K_vmblock_t *)vmb)->intinfo);
		}
	}

	for (i = 0; i < SHARED_INTS; i++) {
		/* There shouldn't be any interrupt pending at this point, but you never
			 know.  Better make the context pointer valid in case we attempt to
			 deliver the interrupt */

		__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
		H2K_vm_shint_enable(vmb, &(((H2K_vmblock_t *)vmb)->contexts[0]), i, ((H2K_vmblock_t *)vmb)->intinfo);
	}
}

extern void linux_stext();

volatile unsigned int *ss_pub_base = (void *)0xFFC00000;
#define FLL_CTL (0x3c >> 2)
#define FLL_STATUS (0x40 >> 2)
#define GFMUX_CTL (0x30 >> 2)

int main(int argc, char *argv[]) {

	void *vmb;

	ss_pub_base[FLL_CTL] = 0x150800; /* M/N */
	ss_pub_base[FLL_CTL] = 0x150801; /* M/N/EN */
	ss_pub_base[FLL_CTL] = 0x150807; /* M/N/EN/RST/SRC */
	ss_pub_base[FLL_CTL] = 0x150805; /* M/N/EN/SRC */
	while ((ss_pub_base[FLL_STATUS] & 1) == 0) /* wait for FLL */;
	ss_pub_base[GFMUX_CTL] = ss_pub_base[GFMUX_CTL] | 0x0C; /* CLK SRC D */
	PRINTF("Set Up Clocks\n");

	h2_init(0);
	h2_config_setfatal(fatal);
	PRINTF("loadlinux: H2 started\n");

#ifdef LINUX
	PRINTF("linux: start boot\n");

	vmb = vm_setup(LINUX_NUM_VCPU, SHARED_INTS, linux_vmblock_space, linux_offset.raw, 0x1, H2K_ASID_TRANS_TYPE_OFFSET);
	setup_ints(vmb, LINUX_NUM_VCPU);
	linux_vmb = vmb;
	PRINTF("linux: vm set up\n");

#ifndef NO_LOAD
	size_t count;
	unsigned long *ptr = (unsigned long *)LINUX_LOAD_ADDR;
	FILE *file;
	char fname[256] = "vmlinux.bin";

	if (argc > 1) {
		sscanf(argv[1], "%s", fname);
	}

	file = fopen(fname, "r");
	if (file == NULL) FAIL("fopen");

	PRINTF("linux: loading %s to 0x%08x\n", fname, (unsigned int)ptr);
	do {
		count = fread(ptr, sizeof(unsigned long), 0x40000, file);
		ptr += count;
		if (count < 0x40000 && ferror(file)) FAIL("ferror");
	} while (!feof(file));
	PRINTF ("linux: loaded %s\n", fname);
#endif

	/* FIXME: set up I$ prefetch and DMT for linux */
	unsigned long val;
#if __QDSP6_ARCH__ >= 4
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
								0, LINUX_VM_PRIO, vmb) == -1) FAIL("linux vmboot");

	PRINTF ("linux: booted\n");
#endif

#ifdef UCOS
	PRINTF("ucos: start boot\n");
	vmb = vm_setup(UCOS_NUM_VCPU, SHARED_INTS, ucos_vmblock_space, (u32_t)ucos_pmap, 0xffffffff, H2K_ASID_TRANS_TYPE_LINEAR);
	PRINTF("ucos: vm set up\n");

	PRINTF("ucos: loading to 0x%08x from 0x%08x, size 0x%08x\n", (unsigned int)ucos_loadaddr, (unsigned int)ucos_image_start, (unsigned int)(ucos_image_end - ucos_image_start));
	memcpy(ucos_loadaddr, ucos_image_start, ucos_image_end - ucos_image_start);

	PRINTF("ucos: loaded\n");

	if (h2_vmboot(ucos_entry, &ucos_vcpu_stacks[0][VCPU_STACK_SIZE - 1],
								0, UCOS_VM_PRIO, vmb) == -1) FAIL("ucos vmboot");

	PRINTF ("ucos: booted\n");
#endif

	h2_thread_stop();
	return 0; // make gcc happy
}

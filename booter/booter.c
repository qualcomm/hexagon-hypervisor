/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <h2_vm.h>
#include <bootmap_macros.h>
#include "elf.h"

#define MAX_SIZE (1024*1024)
#define NUM_TOTAL_THREADS 32

H2K_offset_t offset = {{
	.size = SIZE_16M,
	.cccc = L1WB_L2C,
	.xwru = URWX,
	.pages = 0
	}};

#define FENCE_LO 0
#define FENCE_HI 0x07000000

#define CHILD_INTERRUPT 14

unsigned char storage[MAX_SIZE] __attribute__((aligned(32)));

void FAIL(const char *str)
{
	printf("FAIL %s\n", str);
	exit(1);
}

unsigned int spawn_vm(void *pc)
{
	unsigned long vm;
	unsigned long ret;

	vm = h2_config_vmblock_init(0,SET_CPUS_INTS,NUM_TOTAL_THREADS,0);

	ret = h2_config_vmblock_init(vm, SET_PMAP_TYPE, (unsigned int)offset.raw, H2K_ASID_TRANS_TYPE_OFFSET);
	if (ret != vm) FAIL("SET_PMAP_TYPE");

	ret = h2_config_vmblock_init(vm, SET_FENCES, FENCE_LO, FENCE_HI);
	if (ret != vm) FAIL("SET_FENCES");

	ret = h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);
	if (ret != vm) FAIL("SET_PRIO_TRAPMASK");

	/* Stats Reset */
	asm volatile (" r0 = #0x48 ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory");
	h2_vmboot(pc,(void *)0x07fffff0,0,0,vm);
	printf("vm booted ID %lu\n", vm);
	return vm;
}

void dcclean_range(unsigned long start, long range)
{
	unsigned long p;
	p = start & -32;
	range += start-p;
	do {
		asm volatile (" dccleana(%0)\n" : :"r"(p));
		p += 32;
		range -= 32;
	} while (range >= 0); 
}

extern void bootvm_vectors();

int main(int argc, char **argv)
{
	int ret,i;
	unsigned long vm;
	FILE *f;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	h2_init(0);
	if (argc < 2) {
		printf("%s <file> ...",argv[0]);
		return 1;
	}

	h2_vmtrap_setvec(bootvm_vectors);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, CHILD_INTERRUPT, 0);
	h2_vmtrap_intop(H2K_INTOP_LOCEN, CHILD_INTERRUPT, 0);

	for (; argc > 1; argc--) {

		f = fopen(argv[argc - 1],"rb");
		if ((ret = elf_get_ehdr(f,&ehdr)) < 0) {
			printf("Invalid ELF file: %s\n", argv[argc - 1]);
			return 1;
		}
		for (i = 0; i < ehdr.e_phnum; i++) {
			if (elf_get_phdr(f,i,&phdr,&ehdr) < 0) continue;
			if (phdr.p_memsz == 0) continue;
			if (phdr.p_type != PT_LOAD) continue;
			fseek(f,phdr.p_offset,SEEK_SET);
			if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;
			fread((char *)phdr.p_paddr,1,phdr.p_filesz,f);
			memset((char *)phdr.p_paddr+phdr.p_filesz,0,phdr.p_memsz-phdr.p_filesz);
			/* Really, only need to clean out text sections */
			dcclean_range(phdr.p_paddr,phdr.p_memsz);
		}
		fclose(f);

		printf("Boot vm for %s\n", argv[argc - 1]);
		vm = spawn_vm((void *)ehdr.e_entry);
	
		do {  // wait for all child VM cpus to vmstop
			h2_vmtrap_wait();
			ret = h2_vmstatus(VMOP_STATUS_STATUS, vm);
			printf("VM %lu status %d\n", vm, ret);
			ret = h2_vmstatus(VMOP_STATUS_CPUS, vm);
			printf("VM %lu Live CPUs: %d\n", vm, ret);
		} while (ret != 0);
		h2_vmfree(vm);
	}

	return 0;
}


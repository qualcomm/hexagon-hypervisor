/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <h2_vm.h>
#include <ctype.h>
#include <bootmap_macros.h>
#include <h2_common_config.h>

#include <fcntl.h>
#include <unistd.h>

#include "elf.h"
#include "../kernel/include/hw.h"

#define CHILD_INTERRUPT 14
#define VM_BEST_PRIO 0

/* VM config */
static unsigned int num_vcpus = 32;
static unsigned int num_shared_ints = 0;
static unsigned int page_size = SIZE_16M;
static unsigned int fence_lo = H2K_GUEST_START;
static unsigned int fence_hi = H2K_GUEST_END;
static unsigned int bestprio = VM_BEST_PRIO;
static unsigned int trapmask = 0xffffffff;

/* VM first CPU */
static void *stack = (void *)(H2K_GUEST_END - 8);
/* static void *stack = (void *)0x7ffffff0; */
static unsigned int arg = 0;
static unsigned int startprio = VM_BEST_PRIO;

/* rebooting */
static unsigned int boots = 1;
static unsigned int expect_status = 0;

static H2K_offset_t offset = {{
	.size = SIZE_16M,
	.cccc = L1WB_L2C,
	.xwru = URWX,
	.pages = 0
	}};

void FAIL(const char *str)
{
	printf("FAIL %s\n", str);
	exit(1);
}

void usage()
{
	printf("Usage:\n");
	printf("  booter <file> <file_args>\n");
	printf("  booter --list <file1> <file2> ...\n");
	printf("  booter --listfile <listfile>\n");
}		

unsigned int spawn_vm(void *pc)
{
	unsigned long vm;
	long ret;
	int i;

	vm = h2_config_vmblock_init(0, SET_CPUS_INTS, num_vcpus, num_shared_ints);

	offset.size = page_size;
	ret = h2_config_vmblock_init(vm, SET_PMAP_TYPE, (unsigned int)offset.raw, H2K_ASID_TRANS_TYPE_OFFSET);
	if (ret != vm) FAIL("SET_PMAP_TYPE");

	ret = h2_config_vmblock_init(vm, SET_FENCES, fence_lo, fence_hi);
	if (ret != vm) FAIL("SET_FENCES");

	ret = h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, bestprio, trapmask);
	if (ret != vm) FAIL("SET_PRIO_TRAPMASK");

	/* set up interrupts */
	for (i = 0; i < num_shared_ints + PERCPU_INTERRUPTS; i++) {
		ret = h2_config_vmblock_init(vm, MAP_PHYS_INTR, i, CONFIG_PHYSINT_CPUID(i, num_vcpus - 1));
		if (ret != vm) FAIL("MAP_PHYS_INTR");
	}

	/* Stats Reset */
	asm volatile (" r0 = #0x48 ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory");
	ret = h2_vmboot(pc, stack, arg, startprio, vm);
	if (ret == -1) FAIL("vmboot");

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

static void set_cmdline(const char *cmdline, int fdesc, const Elf32_Ehdr *ehdr)
{
	char *dst;
	int addr;
	if ((addr=elf_get_symbol(fdesc,"__boot_cmdline__",ehdr)) == -1) {
		printf("__boot_cmdline__ not found.\n");
	} else {
		printf("__boot_cmdline__ found @ 0x%08x\n",addr);
	}
	dst = (char *)addr;
	dst[0] = 0;
	strcpy(dst,cmdline);
	printf("cmdline set to <<%s>>\n",dst);
}

int run_elf(char *elf, char *cmdline)
{
	int ret, status, cpus, i;
	unsigned long vm;
	int fdesc;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;

	fdesc = open(elf,O_RDONLY);
	if ((ret = elf_get_ehdr(fdesc,&ehdr)) < 0) {
		printf("Invalid ELF file: %s\n", elf);
		return 1;
	}
	for (i = 0; i < ehdr.e_phnum; i++) {
		if (elf_get_phdr(fdesc,i,&phdr,&ehdr) < 0) continue;
		if (phdr.p_memsz == 0) continue;
		if (phdr.p_type != PT_LOAD) continue;
		lseek(fdesc,phdr.p_offset,SEEK_SET);
		if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;
		read(fdesc,(char *)phdr.p_paddr,phdr.p_filesz);
		memset((char *)phdr.p_paddr+phdr.p_filesz,0,phdr.p_memsz-phdr.p_filesz);
		/* Really, only need to clean out text sections */
		dcclean_range(phdr.p_paddr,phdr.p_memsz);
	}
	set_cmdline(cmdline,fdesc,&ehdr);
	close(fdesc);
	printf("Boot vm for %s\n", elf);
	vm = spawn_vm((void *)ehdr.e_entry);
	
	do {  // wait for all child VM cpus to vmstop
		h2_vmtrap_wait();
		status = h2_vmstatus(VMOP_STATUS_STATUS, vm);
		printf("VM %lu status %d\n", vm, status);
		cpus = h2_vmstatus(VMOP_STATUS_CPUS, vm);
		printf("VM %lu Live CPUs: %d\n", vm, cpus);
	} while (ret != 0);
	h2_vmfree(vm);

	if (status != expect_status) { // unexpected status
		return 1;
	}

	return 0;
}

#define BUFSIZE 256

static void strip(char *buf)
{
	int i;
	for (i = strlen(buf)-1; i >= 0; i--) {
		if (isspace(buf[i])) {
			buf[i] = 0;
		} else {
			break;
		}
	}
}

static void die_usage()
{
	usage();
	exit(1);
}

int main(int argc, char **argv)
{
	int i, ret = 0;
	FILE *f;

	char buf[BUFSIZE];
	char file[64];
	unsigned int regval;
	buf[0] = 0;
	//Remove booter from cmdline
	argc--;
	argv++;

	if (argc < 1) {
		usage();
		return 1;
	}

	h2_vmtrap_setvec(bootvm_vectors);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, CHILD_INTERRUPT, 0);
	h2_vmtrap_intop(H2K_INTOP_LOCEN, CHILD_INTERRUPT, 0);
	while (1) {
		if (0 == strcmp(argv[0],"--syscfg")) {
			if (argc < 2) die_usage();
			regval = H2K_get_syscfg();
			printf("Old value for syscfg: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_syscfg(regval);
			regval = H2K_get_syscfg();
			printf("New value for syscfg: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--chicken")) {
			if (argc < 2) die_usage();
			regval = H2K_get_chicken();
			printf("Old value for chicken: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_chicken(regval);
			regval = H2K_get_chicken();
			printf("New value for chicken: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--rgdr")) {
			if (argc < 2) die_usage();
			regval = H2K_get_rgdr();
			printf("Old value for rgdr: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_rgdr(regval);
			regval = H2K_get_rgdr();
			printf("New value for rgdr: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--ccr")) {
			if (argc < 2) die_usage();
			regval = H2K_get_ccr();
			printf("Old value for ccr: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_ccr(regval);
			regval = H2K_get_ccr();
			printf("New value for ccr: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--num_vcpus")) {
			if (argc < 2) die_usage();
			num_vcpus = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--num_shared_ints")) {
			if (argc < 2) die_usage();
			num_shared_ints = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--page_size")) {
			if (argc < 2) die_usage();
			page_size = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--fence_lo")) {
			if (argc < 2) die_usage();
			fence_lo = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--fence_hi")) {
			if (argc < 2) die_usage();
			fence_hi = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--bestprio")) {
			if (argc < 2) die_usage();
			bestprio = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--trapmask")) {
			if (argc < 2) die_usage();
			trapmask = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--stack")) {
			if (argc < 2) die_usage();
			stack = (void *)strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--arg")) {
			if (argc < 2) die_usage();
			arg = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--boots")) {
			if (argc < 2) die_usage();
			boots = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--expect_status")) {
			if (argc < 2) die_usage();
			expect_status = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--startprio")) {
			if (argc < 2) die_usage();
			startprio = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--list")) {
			// shift '--list' off arg list
			argc--;
			argv++;
			for (; argc > 0; argc--) {
				run_elf(argv[argc - 1]," ");
			}
			return 0;
		} else if (0 == strcmp(argv[0], "--listfile")) {
			if (argc < 2) die_usage();
			if ((f = fopen(argv[1],"r")) == NULL) die_usage();
			while (fgets(buf,BUFSIZE,f)) {
				strip(buf);
				if (sscanf(buf,"%s ",file) <= 0) {
					continue;
				}
				run_elf(file,buf);
			}
			return 0;
		} else {
			break;
		}
	}

	for (i = 0; i < argc; i++) {
		strcat(buf,argv[i]);
		strcat(buf," ");
	}

	while (boots-- && ret == 0) {
		ret = run_elf(argv[0],buf);
	}
	return ret;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <h2.h>
#include <stdio.h>
#include <errno.h>
#include <angel.h>
#include <stdlib.h>
#include <string.h>
#include <h2_vm.h>
#include <ctype.h>
#include <h2_common_pmap.h>
#include <h2_common_config.h>

#include <fcntl.h>
#include <unistd.h>

#include "elf.h"
#include <hw.h>

#define CHILD_INTERRUPT 14
#define VM_BEST_PRIO 0

/* VM config */
static unsigned int num_vcpus = 32;
#ifdef HAVE_EXTENSIONS
static unsigned int use_ext = 0;
#endif
static unsigned int num_shared_ints = 0;
static unsigned int page_size = SIZE_16M;
static unsigned int offset_pages = 0;
static int trans_type = -1;
static unsigned int fence_lo = H2K_GUEST_START;
static unsigned int fence_hi = H2K_GUEST_END;
static long load_offset = -1;
static unsigned int skip_load = 0;
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
		.size = SIZE_4M,
		.cccc = L1WB_L2C,
		.xwru = URWX,
		.pages = 0
	}};

/* Misc */
#define ERRSTR_LEN 1024
static char errstr[ERRSTR_LEN];

void error(char *str1, char *str2) {

	int err = sys_errno();
	strncat(errstr, ": ", ERRSTR_LEN - strlen(errstr));
	strncat(errstr, str1, ERRSTR_LEN - strlen(errstr));
	strncat(errstr, str2, ERRSTR_LEN - strlen(errstr));
	errno = err;
	perror(errstr);
}

void FAIL(const char *str)
{
	printf("FAIL %s\n", str);
	exit(1);
}

void usage()
{
	printf("Usage:\n");
	printf("  booter [options] <file> <file_args>\n");
	printf("  booter [options] --list <file1> <file2> ...\n");
	printf("  booter [options] --listfile <listfile>\n");
	printf("\nConfig options:\n");
	printf("  --chicken <int>\n\tSet the chicken bits.\n");
	printf("  --ccr <int>\n\tSet ccr.\n");
	printf("  --rgdr <int>\n\tSet rgdr.\n");
	printf("  --syscfg <int>\n\tSet syscfg.\n");
	printf("\nVM options:\n");
	printf("  --num_vcpus <int>\n\tMax number of virtual CPUs.\n");
	printf("  --use_ext (0|1)\n\tSupport extended contexts\n");
	printf("  --num_shared_ints <int>\n\tNumber of shared interrupts.\n");
	printf("  --page_size [ 0 == 4K, 1 == 16K, 2 == 64K, 3 == 256K, 4 == 1M, 5 == 4M, 6 == 16M ]\n\tEncoded page size for guest->phys offset map. Default 6 (16M).\n");
	printf("  --offset_pages <int>\n\tOffset (in number of pages) for guest->phys offset map. Default 0.\n");
	printf("  --translation_type <int>\n\tTranslation type for guest->phys map. Default OFFSET (only OFFSET works from cmdline right now).\n");
	printf("  --fence_lo <int>\n\tLowest physical page accessible by guest VM. Must be page_size-aligned. Default 0x01000000.\n");
	printf("  --fence_hi <int>\n\tHighest physical page accessible by guest VM. Must be page_size-aligned. Default 0xff000000.\n");
	printf("  --load_offset <int>\n\tOffset for loading ELF image. Default (0x01000000 - <first_program_header_addr>).\n");
	printf("  --skip_load (0|1)\n\tSkip program loading (e.g. if loaded by simulator with --extra_elf)\n");
	printf("  --bestprio <int>\n\tBest allowed priority for a virtual CPU. Default 0.\n");
	printf("  --trapmask <int>\n\tBitmask of allowed trap0 numbers. Default 0xffffffff (all allowed).\n");
	printf("  --stack <int>\n\tStack pointer VA for first virtual CPU. Default 0xfeffff8.\n");
	printf("  --arg <int>\n\tInitial argument (R0) for first virtual CPU. Default 0.\n");
	printf("  --boots <int>\n\tNumber of times to boot the VM, if exiting with expected status. Default 1.\n");
	printf("  --expect_status <int>\n\tReboot-request status value. The last virtual CPU is expected to vmstop with this status, in which case the VM is started again if the requested number of boots has not been reached. Default 0.\n");
	printf("  --startprio <int>\n\tInitial priority of first virtual CPU. Default 0.\n");

}		

static h2_guest_pmap_t *get_pmap(int fdesc, const Elf32_Ehdr *ehdr) {

	int addr;

	if ((addr = elf_get_symbol(fdesc, "__guest_pmap__", ehdr)) == -1) {
		printf("__guest_pmap__ not found.\n");
		return 0;
	} else {
		printf("__guest_pmap__ found @ 0x%08x\n",addr);
	}

	return (h2_guest_pmap_t *)addr;
}

static void set_cmdline(const char *cmdline, int fdesc, const Elf32_Ehdr *ehdr)
{
	char *dst;
	int addr;
	if ((addr=elf_get_symbol(fdesc,"__boot_cmdline__",ehdr)) == -1) {
		printf("__boot_cmdline__ not found.\n");
		return;
	} else {
		printf("__boot_cmdline__ found @ 0x%08x\n",addr);
	}
	dst = (char *)addr;
	dst[0] = 0;
	strcpy(dst,cmdline);
	printf("cmdline set to <<%s>>\n",dst);
}

unsigned int spawn_vm(int fdesc, const Elf32_Ehdr *ehdr, long phys_offset)
{
	unsigned long vm;
	long ret;
	int i;
	h2_guest_pmap_t *pmap;
	void *pc = (void *)ehdr->e_entry - phys_offset + load_offset;
	H2K_offset_t base;
	int trans;

	printf("pc %08lx\n", (unsigned long)pc);

	vm = h2_config_vmblock_init(0, SET_CPUS_INTS, CONFIG_CPUS(use_ext, num_vcpus), num_shared_ints);

	if (trans_type == -1) { // not set on cmdline, get from guest image
		pmap = get_pmap(fdesc, ehdr);

		if (pmap != NULL) { // found
			trans = pmap->type;
			base.raw = pmap->base.raw;
		} else { // default
			trans = H2K_ASID_TRANS_TYPE_OFFSET;
			base.raw = offset.raw;
			base.size = page_size;
			base.pages = offset_pages;
		}
	} else { // translation type forced; better only be offset for now
		if (trans_type != H2K_ASID_TRANS_TYPE_OFFSET) {
			printf("Are you really going to type page tables on the command line?\n");
			exit(1);
		}
		trans = trans_type;
		base.raw = offset.raw;
		base.size = page_size;
		base.pages = offset_pages;
	}

	ret = h2_config_vmblock_init(vm, SET_PMAP_TYPE, (unsigned int)base.raw, trans);
	if (ret != vm) FAIL("SET_PMAP_TYPE");

	if (trans == H2K_ASID_TRANS_TYPE_OFFSET) {
		ret = h2_config_vmblock_init(vm, SET_FENCES, fence_lo, fence_hi);
		if (ret != vm) FAIL("SET_FENCES");
	}

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

int run_elf(char *elf, char *cmdline)
{
	int ret, status, cpus, i;
	unsigned long vm;
	int fdesc;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	long phys_offset = -1;
	int bytes_read;

	fdesc = open(elf,O_RDONLY);
	if (fdesc == -1) {
		error("Can't open file ", elf);
		return 1;
	}
	if (elf_get_ehdr(fdesc,&ehdr) < 0) {
		printf("Invalid ELF file: %s\n", elf);
		return 1;
	}
	for (i = 0; i < ehdr.e_phnum; i++) {
		if (elf_get_phdr(fdesc,i,&phdr,&ehdr) < 0) continue;
		if (phdr.p_memsz == 0) continue;
		if (phdr.p_type != PT_LOAD) continue;
		if (lseek(fdesc,phdr.p_offset,SEEK_SET) == -1) {
			error("Can't lseek() in ", elf);
			return 1;
		}
		if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;

		/* FIXME: Assuming first program header contains entry point*/
		if (phys_offset == -1) { //unset
			phys_offset = phdr.p_vaddr - phdr.p_paddr;
			printf("phys_offset %lx\n", phys_offset);
		}
		/* FIXME: Assuming prog headers in sorted order.  Override with --load_offset if needed */
		if (load_offset == -1) {
			load_offset = H2K_GUEST_START - phdr.p_paddr;
			printf("load_offset %lx\n", load_offset);
		}
		phdr.p_paddr += load_offset;

		if (!skip_load) {
			printf("load VA %08lx at %08lx\n", (unsigned long)phdr.p_vaddr, (unsigned long)phdr.p_paddr);
			bytes_read = 0;
			do {
				bytes_read += ret = read(fdesc,(char *)phdr.p_paddr + bytes_read, phdr.p_filesz - bytes_read);
			} while (ret > 0);
			if (ret == -1) FAIL("read()");

			memset((char *)phdr.p_paddr+phdr.p_filesz, 0, phdr.p_memsz-phdr.p_filesz);
			/* Really, only need to clean out text sections */
			dcclean_range(phdr.p_paddr, phdr.p_memsz);
		}
	}
	set_cmdline(cmdline,fdesc,&ehdr);
	printf("Boot vm for %s\n", elf);
	vm = spawn_vm(fdesc, &ehdr, phys_offset);
	close(fdesc);
	
	do {  // wait for all child VM cpus to vmstop
		h2_vmtrap_setie(0);
		printf("Waiting for child interrupt\n");
		h2_vmtrap_wait();
		h2_vmtrap_setie(1); // take the interrupt to clear it
		status = h2_vmstatus(VMOP_STATUS_STATUS, vm);
		printf("VM %lu status %d\n", vm, status);
		cpus = h2_vmstatus(VMOP_STATUS_CPUS, vm);
		printf("VM %lu Live CPUs: %d\n", vm, cpus);
	} while (cpus != 0);
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
	strncpy(errstr, argv[0], ERRSTR_LEN);
	argc--;
	argv++;

	if (argc < 1) {
		usage();
		return 1;
	}

	h2_vmtrap_setvec(bootvm_vectors);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, CHILD_INTERRUPT, 0);
	//h2_vmtrap_intop(H2K_INTOP_LOCEN, CHILD_INTERRUPT, 0);
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

#ifdef HAVE_EXTENSIONS
		} else if (0 == strcmp(argv[0], "--use_ext")) {
			if (argc < 2) die_usage();
			use_ext = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
#endif

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

		} else if (0 == strcmp(argv[0], "--offset_pages")) {
			if (argc < 2) die_usage();
			offset_pages = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--translation_type")) {
			if (argc < 2) die_usage();
			trans_type = strtoul(argv[1],NULL,0);
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

		} else if (0 == strcmp(argv[0], "--load_offset")) {
			if (argc < 2) die_usage();
			load_offset = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--skip_load")) {
			if (argc < 2) die_usage();
			skip_load = strtoul(argv[1],NULL,0);
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

		} else if (0 == strcmp(argv[0], "--help")) {
			usage();
			exit(0);
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

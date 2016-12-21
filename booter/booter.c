/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//#include <max.h>
#include <h2.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <angel.h>
#include <stdlib.h>
#include <string.h>
#include <h2_vm.h>
#include <ctype.h>
#include <h2_common_pmap.h>
#include <h2_common_config.h>
#include <h2_common_error.h>
#include <h2_common_defs.h>
#include <h2_kerror.h>
#include <h2_alloc.h>
#include <h2_common_linear.h>
#include <h2_sleep.h>
#include <h2_prof.h>

#include <fcntl.h>
#include <unistd.h>

#include "elf.h"
#include "../kernel/include/max.h"
#include "../kernel/include/hw.h"
#include <syscall_defs.h>

#define VM_BEST_PRIO 0

#define BUFSIZE 256

/* Misc */
#define ERRSTR_LEN 1024
char errstr[ERRSTR_LEN];

#define FOREACH_sym(GEN)												\
	GEN(__guest_pmap__)														\
	GEN(__boot_cmdline__)													\
	GEN(__boot_net_phys_offset__)									\
	GEN(__use_dir_prefix__)												\
	GEN(__use_file_suffix__)											\
	GEN(__dir_prefix__)														\
	GEN(__file_suffix__)													\
	GEN(end)																			\
	GEN(DEFAULT_HEAP_SIZE)												\
	GEN(DEFAULT_STACK_SIZE)												\
	GEN(HEAP_SIZE)																\
	GEN(STACK_SIZE)																\
	GEN(__clade_region_high_pd0_start__)					\
	GEN(__clade_comp_pd0_start__)									\
	GEN(__clade_exception_high_pd0_start__)				\
	GEN(__clade_exception_high_pd0_end__)					\
	GEN(__clade_exception_low_small_pd0_start__)	\
	GEN(__clade_exception_low_large_pd0_start__)  \
	GEN(__clade_dict_pd0_start__)

#define GEN_enum(NAME) SPECIAL_ ## NAME,
enum {
	FOREACH_sym(GEN_enum)
	SPECIAL_NUM_ENTRIES
};

#define HI_MASK(SIZ) (-(SIZ))

#define GUESS_HEAP_SIZE 0x4000000 /* 64MB */
#define GUESS_STACK_SIZE 0x100000 /* 1MB */

#define FENCE_HI_MAX 0xfffff000

#define WAKE_TIMER 0x1
#define WAKE_CHILD 0x2

/* Globals */
#ifdef HAVE_EXTENSIONS
unsigned int ext_power = 1;
#endif
unsigned int use_stlb = 0;
unsigned int tight_fence_hi = 0;
unsigned long guest_base = H2K_GUEST_START;
h2_anysignal_t wake_sig;
int tcm_base;
int tcm_size;
h2_galloc_t tcm_alloc;
int clade_base;
int pd_num = 0;
unsigned long clade_region = 0;
unsigned int sample_usecs = 0;
info_boot_flags_type boot_flags;
info_stlb_type  stlb_info;

typedef struct {
	unsigned int id;  // h2 VM id
	int cloneof;      // index that was cloned

	/* VM config */
	unsigned int num_vcpus;
#ifdef HAVE_EXTENSIONS
	unsigned int use_ext;
#endif
	unsigned int num_shared_ints;
	unsigned int ccr;

	/* Translation options from cmdline */
	unsigned int page_size;
	unsigned int cccc;
	unsigned int xwru;
	long offset_pages;
	int trans_type;

	unsigned int fence_lo;
	unsigned int fence_hi;
	long load_offset;
	long phys_offset;
	unsigned long long start_pa;
	unsigned long long end_pa;
	unsigned long start_va;
	unsigned long end_va;
	unsigned int pages;

	unsigned int skip_load;
	unsigned int bestprio;
	unsigned int trapmask;

	/* VM first CPU */
	void *entry;
	void *stack;
	unsigned int arg;
	unsigned int startprio;

	/* rebooting */
	unsigned int boots;
	unsigned int expect_status;

	/* exit on error */
	unsigned int error_exit;

	/* clade */
	int clade_pd;
	unsigned int clade_ex_hi;

	special_symbols specials[SPECIAL_NUM_ENTRIES];

	/* From __guest_pmap__ */
	h2_guest_pmap_t *pmap;
	int pmap_added;

	char **argv;
	int argc;

	int use_dir_prefix;
	int use_file_suffix;
	char *dir_prefix;
	char *file_suffix;
} vm_t;

vm_t *vm_params = NULL;

void error(char *str1, char *str2) {

	int err = sys_errno();
	strncat(errstr, ": ", ERRSTR_LEN - strlen(errstr) - 1);
	strncat(errstr, str1, ERRSTR_LEN - strlen(errstr) - 1);
	strncat(errstr, str2, ERRSTR_LEN - strlen(errstr) - 1);
	errno = err;
	perror(errstr);

	exit(1);
}

void FAIL(const char *str1, const char *str2)
{
	printf("FAIL %s %s\n", str1, str2);
	exit(1);
}

void usage()
{
	printf("Usage:\n");
	printf("  booter [options] <executable> <args>\n");
	printf("  booter [options] <executable> <args> [ --new_vm <instances> [vm options] <executable> <args> ...]\n");
	printf("  booter [global options] --new_vm <int> [vm options] <executable> <args> [--new_vm <int> [vm options] <executable> <args> ...]\n\tBoot new <instances> of guest VMs with given options.\n");
	printf("  booter --file <path> [--file <path> ...]\n\tOptions from <path>, one guest VM per line.");
	printf("\n");
	printf("Global options:\n");
	printf("  --duck <int>\n\tSet the duck bits.\n");
	printf("  --chicken <int>\n\tSet the chicken bits.\n");
	printf("  --rgdr <int>\n\tSet rgdr.\n");
	printf("  --syscfg <int>\n\tSet syscfg.\n");
	printf("  --livelock <int>\n\tSet livelock.\n");
	printf("  --syscfg_bit <name> <int>\n\tSet syscfg bit(s) not covered by other options.\n");
	printf("  --l1dp [ 0 == shared, 1 == 1/2 main, 2 == 3/4 main ]\n\tSet L1 data cache partitioning (ARCHV <= 5).\n");
	printf("  --l1ip [ 0 == shared, 1 == 1/2 main, 2 == 3/4 main ]\n\tSet L1 instruction cache partitioning (ARCHV <= 5).\n");
	printf("  --l2part [ 0 == shared, 1 == 1/2 main, 2 == 3/4 main, 3 == 7/8 main ]\n\tSet L2 cache partitioning.\n");
	printf("  --l2cfg <int>\n\tSet L2 cache tag size bits.\n");
	printf("  --l2_reg <offset int> <int>\n\tSet L2 config register. Setting to -1 reads current value, doesn't set.\n");
#ifdef HAVE_EXTENSIONS
	printf("  --ext_power (0|1)\n\tPower on coprocessor.  Default 1.\n");
#endif
	printf("  --use_stlb (0|1)\n\tTurn on STLB.  Default 0.\n");
	printf("  --guest_base <int>\n\tStart of guest physical memory. Default 0x%08x.\n", H2K_GUEST_START);
	printf("  --sample <int>\n\tSet guest PC sample interval in usecs. Default 0 (disabled).\n");

	printf("\n");
	printf("VM options:\n");
	printf("  --ccr <int>\n\tSet ccr.\n");
	printf("  --num_vcpus <int>\n\tMax number of virtual CPUs. Default 32.\n");
#ifdef HAVE_EXTENSIONS
	printf("  --use_ext (0|1)\n\tSupport extended contexts.  Default 0.\n");
#endif
	printf("  --num_shared_ints <int>\n\tNumber of shared interrupts.  Default 0.\n");
	printf("  --page_size [ 0 == 4K, 1 == 16K, 2 == 64K, 3 == 256K, 4 == 1M, 5 == 4M, 6 == 16M ]\n\tEncoded page size for guest->phys offset map.  Default 6 (16M).\n");
	printf("  --cccc <int>\n\tCache bits for guest->phys offset map.  Default L1WB_L2C (0xa == L1WB_L2CWB_AUX).\n");
	printf("  --offset_pages <int>\n\tOffset (in number of pages) for guest->phys offset map.  Default matches load_offset, or 0.\n");
	printf("  --translation_type [ %d == OFFSET ]\n\tTranslation type for guest->phys map.  Default OFFSET (only OFFSET works from cmdline right now.  Used to override guest_pmap).\n", H2K_ASID_TRANS_TYPE_OFFSET);
	printf("  --fence_lo <int>\n\tLowest physical page accessible by guest VM.  Must be page_size-aligned.  Default lowest mapped physical page.\n");
	printf("  --fence_hi <int>\n\tHighest physical page accessible by guest VM.  Must be page_size-aligned.  Default (end - fence_lo) + heap size + stack size.\n");
	printf("  --load_offset <int>\n\tOffset for loading ELF image.  Default (guest_base - <first_program_header_addr>).\n");
	printf("  --skip_load (0|1)\n\tSkip program loading (e.g. if loaded by simulator with --extra_elf).  Default 0.\n");
	printf("  --bestprio <int>\n\tBest allowed priority for a virtual CPU.  Default 0.\n");
	printf("  --trapmask <int>\n\tBitmask of allowed trap0 numbers.  Default 0xffffffff (all allowed).\n");
	printf("  --stack <int>\n\tStack pointer VA for first virtual CPU.  Default 0xfeffff8.\n");
	printf("  --arg <int>\n\tInitial argument (R0) for first virtual CPU.  Default 0.\n");
	printf("  --boots <int>\n\tNumber of times to boot the VM, if exiting with expected status.  Default 1.\n");
	printf("  --expect_status <int>\n\tReboot-request status value. The last virtual CPU is expected to vmstop with this status, in which case the VM is started again if the requested number of boots has not been reached.  Default 0.\n");
	printf("  --error_exit (0|1)\n\tExit when a virtual CPU stops on fatal error.  Default 1.\n");
	printf("  --startprio <int>\n\tInitial priority of first virtual CPU.  Default 0.\n");
	printf("  --dir_prefix <string>\n\tPrepend <string> to relative paths when opening files. Default null string.\n");
	printf("  --file_suffix <string>\n\tAppend <string> to file names when opening files write-only. Default null string.\n");
}		

#define GEN_specials(NAME) vm_params[idx].specials[SPECIAL_ ## NAME].name = #NAME; vm_params[idx].specials[SPECIAL_ ## NAME].addr = -1;

void add_vm(unsigned int idx) {

	if (NULL == (vm_params = (vm_t *)(realloc((void *)vm_params, sizeof(vm_t) * (idx + 1))))) {
		error("realloc vm_params", NULL);
	}

	vm_params[idx].cloneof = -1;  // not a clone

	vm_params[idx].num_vcpus = 32;
#ifdef HAVE_EXTENSIONS
	vm_params[idx].use_ext = 0;
#endif
	vm_params[idx].num_shared_ints = 0;
	vm_params[idx].ccr = ~0L;

	vm_params[idx].page_size = SIZE_16M;
	vm_params[idx].cccc = L1WB_L2C;
	vm_params[idx].xwru = URWX;
	vm_params[idx].offset_pages = -1;
	vm_params[idx].trans_type = -1;
	vm_params[idx].fence_lo = ~0L;
	vm_params[idx].fence_hi = 0L;
	vm_params[idx].load_offset = -1;
	vm_params[idx].phys_offset = -1;
	vm_params[idx].start_pa = 0ULL;
	vm_params[idx].end_pa = 0ULL;
	vm_params[idx].skip_load = 0;
	vm_params[idx].bestprio = VM_BEST_PRIO;
	vm_params[idx].trapmask = 0xffffffff;
	vm_params[idx].entry = NULL;
	vm_params[idx].stack = NULL;
	vm_params[idx].arg = 0;
	vm_params[idx].startprio = VM_BEST_PRIO;
	vm_params[idx].boots = 1;
	vm_params[idx].expect_status = 0;
	vm_params[idx].error_exit = 1;
	vm_params[idx].clade_pd = -1;
	vm_params[idx].clade_ex_hi = CLADE_INVALID_ADDRESS;

	FOREACH_sym(GEN_specials);

	vm_params[idx].pmap = NULL;
	vm_params[idx].pmap_added = 0;

	vm_params[idx].argv = NULL;
	vm_params[idx].argc = 0;

	vm_params[idx].use_dir_prefix = 0;
	vm_params[idx].use_file_suffix = 0;
	vm_params[idx].dir_prefix = "";
	vm_params[idx].file_suffix = "";
}

#undef GEN_specials
#define GEN_specials(NAME) vm_params[idx + num].specials[SPECIAL_ ## NAME].name = #NAME; // vm_params[idx + num].specials[SPECIAL_ ## NAME].addr = vm_params[idx].specials[SPECIAL_ ## NAME].addr;

void clone_vm(unsigned int idx, unsigned int num) {

	if (NULL == (vm_params = (vm_t *)(realloc((void *)vm_params, sizeof(vm_t) * (idx + 1 + num))))) {
		FAIL("realloc vm_params", "");
	}

	while (num) {
		vm_params[idx + num].cloneof = idx;

		vm_params[idx + num].num_vcpus = vm_params[idx].num_vcpus;
#ifdef HAVE_EXTENSIONS
		vm_params[idx + num].use_ext = vm_params[idx].use_ext;
#endif
		vm_params[idx + num].num_shared_ints = vm_params[idx].num_shared_ints;
		vm_params[idx + num].ccr = ~0L;
		vm_params[idx + num].page_size = vm_params[idx].page_size;
		vm_params[idx + num].cccc = vm_params[idx].cccc;
		vm_params[idx + num].xwru = vm_params[idx].xwru;
		//		vm_params[idx + num].offset_pages = vm_params[idx].offset_pages;
		vm_params[idx + num].trans_type = vm_params[idx].trans_type;
		//		vm_params[idx + num].fence_lo = ~0L;
		//		vm_params[idx + num].fence_hi = 0L;
		//		vm_params[idx + num].load_offset = vm_params[idx].load_offset;
		//		vm_params[idx + num].phys_offset = vm_params[idx].phys_offset;
		//		vm_params[idx + num].start_pa = 0ULL;
		//		vm_params[idx + num].end_pa = 0ULL;
		vm_params[idx + num].skip_load = vm_params[idx].skip_load;
		vm_params[idx + num].bestprio = vm_params[idx].bestprio;
		vm_params[idx + num].trapmask = vm_params[idx].trapmask;
		//		vm_params[idx + num].entry = vm_params[idx].entry;
		//		vm_params[idx + num].stack = vm_params[idx].stack;
		vm_params[idx + num].arg = vm_params[idx].arg;
		vm_params[idx + num].startprio = vm_params[idx].startprio;
		vm_params[idx + num].boots = vm_params[idx].boots;
		vm_params[idx + num].expect_status = vm_params[idx].expect_status;
		vm_params[idx + num].error_exit = vm_params[idx].error_exit;
		vm_params[idx + num].clade_pd = -1;
		vm_params[idx + num].clade_ex_hi = CLADE_INVALID_ADDRESS;

		FOREACH_sym(GEN_specials);

		vm_params[idx + num].pmap = vm_params[idx].pmap;
		vm_params[idx + num].pmap_added = 0;

		vm_params[idx + num].argv = vm_params[idx].argv;
		vm_params[idx + num].argc = vm_params[idx].argc;

		//		vm_params[idx + num].use_dir_prefix = vm_params[idx].use_dir_prefix;
		//		vm_params[idx + num].use_file_suffix = vm_params[idx].use_file_suffix;
		//		vm_params[idx + num].dir_prefix = vm_params[idx].dir_prefix;
		//		vm_params[idx + num].file_suffix = vm_params[idx].file_suffix;

		num--;
	}
}

void get_pmap(unsigned int idx, long offset) {

	unsigned long addr;

	if ((addr = vm_params[idx].specials[SPECIAL___guest_pmap__].addr) == -1) {
		printf("\t__guest_pmap__ not found.\n");
	} else {
		printf("\t__guest_pmap__ found @ 0x%08lx\n",addr);

		vm_params[idx].pmap = (h2_guest_pmap_t *)(addr + offset);
	}
}

void dcclean_range(unsigned long start, long range) {

	unsigned long p;
	p = start & -32;
	range += start-p;
	do {
		asm volatile (" dccleana(%0)\n" : :"r"(p));
		p += 32;
		range -= 32;
	} while (range >= 0); 
}

void set_cmdline(unsigned int idx, long offset) {

	int i;
	char *dst;
	unsigned long addr;
	int len = 0;

	if ((addr = vm_params[idx].specials[SPECIAL___boot_cmdline__].addr) == -1) {
		printf("\t__boot_cmdline__ not found.\n");
		return;
	} else {
		printf("\t__boot_cmdline__ found @ 0x%08x\n", (unsigned int)addr);
	}
	dst = (char *)(addr + offset);
	dst[0] = 0;
	for (i = 0; i < vm_params[idx].argc; i++) {
		if ((len += strlen(vm_params[idx].argv[i])) >= SIZE__boot_cmdline__) {
			FAIL("__boot_cmdline__: string too long; can't strcat ", vm_params[idx].argv[i]);
		}
		strcat(dst, vm_params[idx].argv[i]);
		strcat(dst, " ");
	}

	printf("\tcmdline at 0x%08x set to <<%s>>\n", (unsigned int)dst, dst);
}

void set_string(unsigned int idx, int sym, char *string, int maxlen, long offset) {

	char *dst;
	unsigned long addr;

	if (NULL == string) return;

	if (strlen(string) >= maxlen - 1) {
		FAIL(vm_params[idx].specials[sym].name, ": string too long");
	}

	if ((addr = vm_params[idx].specials[sym].addr) == -1) {
		printf("\t%s not found.\n", vm_params[idx].specials[sym].name);
		return;
	} else {
		printf("\t%s found @ 0x%08x\n", vm_params[idx].specials[sym].name, (unsigned int)addr);
	}

	dst = (char *)(addr + offset);
	strcpy(dst, string);

	printf("\t%s at 0x%08x set to <<%s>>\n", vm_params[idx].specials[sym].name, (unsigned int)dst, dst);
}

void set_var(unsigned int idx, int sym, int val, long offset) {

	int *dst;
	unsigned long addr;

	if ((addr = vm_params[idx].specials[sym].addr) == -1) {
		printf("\t%s not found.\n", vm_params[idx].specials[sym].name);
		return;
	} else {
		printf("\t%s found @ 0x%08x\n", vm_params[idx].specials[sym].name, (unsigned int)addr);
	}

	dst = (int *)(addr + offset);
	*dst = val;

	printf("\t%s at 0x%08x set to <<%d>>\n", vm_params[idx].specials[sym].name, (unsigned int)dst, *dst);
}

void set_net_phys_offset(unsigned int idx, long offset) {

	long *dst;
	unsigned long addr;
	if ((addr = vm_params[idx].specials[SPECIAL___boot_net_phys_offset__].addr) == -1) {
		printf("\t__boot_net_phys_offset__ not found.\n");
		return;
	} else {
		printf("\t__boot_net_phys_offset__ found @ 0x%08x\n", (unsigned int)addr);
	}
	dst = (long *)(addr + offset);
	*dst = offset;

	printf("\tnet phys offset at 0x%08x set to <<0x%08x>>\n", (unsigned int)dst, (unsigned int)*dst);
}

void add_linear_trans(unsigned int idx, unsigned long va, unsigned long long pa, int page_size, int npages) {
	h2_guest_pmap_t *pmap = vm_params[idx].pmap;
	H2K_linear_fmt_t *base;
	int end = 0;
	int i;

	if (NULL != pmap) {
		if (!vm_params[idx].pmap_added) {
			FAIL("add_linear_trans to existing pmap", "");
		}

		base = (H2K_linear_fmt_t *)(pmap->base.raw);
		while (0ULL != base[end].raw) {
			end++;
		}
	} else {
		if (NULL == (pmap = vm_params[idx].pmap = (h2_guest_pmap_t *)malloc(sizeof(h2_guest_pmap_t)))) {
			error("malloc pmap", NULL);
		}
		pmap->type = H2K_ASID_TRANS_TYPE_LINEAR;
		pmap->base.raw = 0;
		vm_params[idx].pmap_added = 1;
	}

	if (NULL == (pmap->base.raw = (h2_u32_t)realloc((void *)(pmap->base.raw), sizeof(H2K_linear_fmt_t) * (end + npages + 1)))) {
		error("realloc pmap->base", NULL);
	}
	base = (H2K_linear_fmt_t *)(pmap->base.raw);

	/* append translations */
	va >>= H2K_KERNEL_ADDRBITS;
	pa >>= H2K_KERNEL_ADDRBITS;
		
	for (i = 0; i < npages; i++) {
		//		printf("trans 0x%08lx -> 0x%09llx\n", va << H2K_KERNEL_ADDRBITS, pa << H2K_KERNEL_ADDRBITS);
		base[end + i].raw = 0ULL;
		base[end + i].ppn = pa;
		base[end + i].cccc = L1WB_L2C;
		base[end + i].xwru = URWX;
		base[end + i].vpn = va;
		base[end + i].size = page_size;

		va += 1 << (page_size * 2);
		pa += 1 << (page_size * 2);
	}
	base[end + i].raw = 0LL;  // end marker
}

void clade_setup(unsigned int idx, long offset) {

	unsigned long region_hi, comp, ex_lo_small, ex_lo_large, ex_hi_start, ex_hi_end, ex_hi_size, dict_start;
	h2_u32_t *from, *to;
	h2_u32_t tmp;
	int i;

	/* Skip if any clade symbols are missing */
	if ((region_hi = vm_params[idx].specials[SPECIAL___clade_region_high_pd0_start__].addr) == -1) {
		printf("\t__clade_region_high_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_region_high_pd0_start__ found @ 0x%08x\n", (unsigned int)region_hi);
	}
	if (0 == region_hi) {  // unused weak symbol
		goto no_clade;
	}

	if ((comp = vm_params[idx].specials[SPECIAL___clade_comp_pd0_start__].addr) == -1) {
		printf("\t__clade_comp_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_comp_pd0_start__ found @ 0x%08x\n", (unsigned int)comp);
	}

	if ((ex_lo_small = vm_params[idx].specials[SPECIAL___clade_exception_low_small_pd0_start__].addr) == -1) {
		printf("\t__clade_exception_low_small_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_exception_low_small_pd0_start__ found @ 0x%08x\n", (unsigned int)ex_lo_small);
	}

	if ((ex_lo_large = vm_params[idx].specials[SPECIAL___clade_exception_low_large_pd0_start__].addr) == -1) {
		printf("\t__clade_exception_low_large_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_exception_low_large_pd0_start__ found @ 0x%08x\n", (unsigned int)ex_lo_large);
	}

	if ((ex_hi_start = vm_params[idx].specials[SPECIAL___clade_exception_high_pd0_start__].addr) == -1) {
		printf("\t__clade_exception_high_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_exception_high_pd0_start__ found @ 0x%08x\n", (unsigned int)ex_hi_start);
	}

	if ((ex_hi_end = vm_params[idx].specials[SPECIAL___clade_exception_high_pd0_end__].addr) == -1) {
		printf("\t__clade_exception_high_pd0_end__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_exception_high_pd0_end__ found @ 0x%08x\n", (unsigned int)ex_hi_end);
	}

	if ((dict_start = vm_params[idx].specials[SPECIAL___clade_dict_pd0_start__].addr) == -1) {
		printf("\t__clade_dict_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		printf("\t__clade_dict_pd0_start__ found @ 0x%08x\n", (unsigned int)dict_start);
	}

	/* Allocate a clade pd */
	vm_params[idx].clade_pd = pd_num++;
	if (pd_num > CLADE_NUM_PDS) {
		FAIL("\tOut of CLADE pds", "");
	}

	/* Copy high-prio exception data to TCM */
	ex_hi_size = ex_hi_end - ex_hi_start;
	if (ex_hi_size) {
		if (NULL == (vm_params[idx].clade_ex_hi = (unsigned int)h2_galloc(&tcm_alloc, ex_hi_size, 4096, 0))) {
			error("galloc ex_hi", NULL);
		}
		//		printf("memcpy(0x%08x, 0x%08lx, 0x%08lx\n", vm_params[idx].clade_ex_hi, ex_hi_start + offset, ex_hi_size);
		memcpy((void *)(vm_params[idx].clade_ex_hi), (void *)ex_hi_start + offset, ex_hi_size);
	} else {
		vm_params[idx].clade_ex_hi = ex_hi_start + offset;  // should never be referenced, but point it at something kind of meaningful
	}

	/* g->p translations for program */
	add_linear_trans(idx, vm_params[idx].start_va, vm_params[idx].start_pa, vm_params[idx].page_size, vm_params[idx].pages);

	/* g->p translations for clade region */
	add_linear_trans(idx, region_hi, region_hi + CLADE_REGION_LEN * vm_params[idx].clade_pd, SIZE_16M, CLADE_REGION_LEN / 0x01000000);

	/* Set up clade regs */
	h2_hwconfig_clade_set_reg(vm_params[idx].clade_pd * CLADE_REG_PD_CHUNK + CLADE_REG_COMP, comp + offset);
	h2_hwconfig_clade_set_reg(vm_params[idx].clade_pd * CLADE_REG_PD_CHUNK + CLADE_REG_EX_HI, vm_params[idx].clade_ex_hi);
	h2_hwconfig_clade_set_reg(vm_params[idx].clade_pd * CLADE_REG_PD_CHUNK + CLADE_REG_EX_LO_SMALL, ex_lo_small + offset);
	h2_hwconfig_clade_set_reg(vm_params[idx].clade_pd * CLADE_REG_PD_CHUNK + CLADE_REG_EX_LO_LARGE, ex_lo_large + offset);

	/* Copy dictionaries */
	if (0 == vm_params[idx].clade_pd) {  // only copy dicts for first pd; any others had better be identical to these

		/* Can't memcpy here; need to force word accesses */
		from = (h2_u32_t *)dict_start;
		to = (h2_u32_t *)(clade_base + CLADE_DICT_OFFSET);

		for (i = 0; i < CLADE_DICT_LEN * CLADE_NUM_DICTS; i += 4, from++, to++) {
			asm volatile (" %0 = memw(%1); memw(%2) = %0 \n" : "=&r"(tmp) : "r"(from), "r"(to) : "memory");
		}

		h2_hwconfig_clade_set_reg(CLADE_REG_REGION, region_hi);
		clade_region = region_hi;
		H2K_set_syscfg(h2_info(INFO_SYSCFG) | SYSCFG_CLADEN);  // enable clade
	} else if (region_hi != clade_region) {  // has to be identical for all concurrent clade guests
			FAIL("\t CLADE region address mismatch", "");
	}
	printf("\tCLADE enabled\n");
	return;
 no_clade:
	printf("\tCLADE not enabled\n");
}

void load_vm(unsigned int idx) {

	int fdesc, i, ret;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	int bytes_read;

	int set_fence_lo = (~0L == vm_params[idx].fence_lo);
	int set_fence_hi = (0L == vm_params[idx].fence_hi);

	unsigned long heap_size, stack_size, total_size, prev_size, end, one_page, page_shift;
	unsigned long start = ~0L;
	int clone;
	long total_offset;

	char *elf = vm_params[idx].argv[0];

	printf("\n");  // FIXME: prepending \n to string results in an empty line in the output lately. Weird.
	printf("Load VM index %d %s\n", idx, elf);

	/* FIXME? It would be better to get the page size from the __guest_pmap__ if
		 it exists (and if it is an offset mapping), but to read that we need to
		 load first, and to load we need to align guest_base to the page size. It's
		 sufficient to use a page size that is as least as big as the one in the
		 __guest_pmap__; at most we waste some space. */
	page_shift = ((vm_params[idx].page_size * 2) + H2K_KERNEL_ADDRBITS);
	one_page = 1 << page_shift;

	/* Align the guest base up to the current guest's page size */
	guest_base = H2_ALIGN_UP(guest_base, one_page);

	if (-1 != (clone = vm_params[idx].cloneof)) {  // this is a clone of a VM already loaded
		prev_size = (vm_params[clone].fence_hi - vm_params[clone].fence_lo + one_page) * (idx - clone);

		vm_params[idx].specials[SPECIAL___boot_net_phys_offset__].addr = vm_params[clone].specials[SPECIAL___boot_net_phys_offset__].addr;
		vm_params[idx].specials[SPECIAL___clade_region_high_pd0_start__].addr = vm_params[clone].specials[SPECIAL___clade_region_high_pd0_start__].addr;
		vm_params[idx].specials[SPECIAL___clade_comp_pd0_start__].addr = vm_params[clone].specials[SPECIAL___clade_comp_pd0_start__].addr;
		vm_params[idx].specials[SPECIAL___clade_exception_low_small_pd0_start__].addr = vm_params[clone].specials[SPECIAL___clade_exception_low_small_pd0_start__].addr;
		vm_params[idx].specials[SPECIAL___clade_exception_low_large_pd0_start__].addr = vm_params[clone].specials[SPECIAL___clade_exception_low_large_pd0_start__].addr;
		vm_params[idx].specials[SPECIAL___clade_exception_high_pd0_start__].addr = vm_params[clone].specials[SPECIAL___clade_exception_high_pd0_start__].addr;
		vm_params[idx].specials[SPECIAL___clade_exception_high_pd0_end__].addr = vm_params[clone].specials[SPECIAL___clade_exception_high_pd0_end__].addr;
		vm_params[idx].specials[SPECIAL___clade_dict_pd0_start__].addr = vm_params[clone].specials[SPECIAL___clade_dict_pd0_start__].addr;

		vm_params[idx].entry = vm_params[clone].entry;
		vm_params[idx].stack = vm_params[clone].stack;
		vm_params[idx].phys_offset = vm_params[clone].phys_offset;
		vm_params[idx].load_offset = vm_params[clone].load_offset + prev_size;
		total_offset = vm_params[idx].phys_offset + vm_params[idx].load_offset;

		vm_params[idx].offset_pages = (total_offset) >> (vm_params[idx].page_size * 2);
		vm_params[idx].fence_lo = vm_params[clone].fence_lo + prev_size;
		vm_params[idx].fence_hi = vm_params[clone].fence_hi + prev_size;

		if (NULL != vm_params[clone].pmap && !vm_params[clone].pmap_added) { 
			vm_params[idx].pmap = vm_params[clone].pmap + prev_size;
		}

		printf("\tCopying from VM index %d: 0x%09llx to 0x%09llx size 0x%09llx\n", clone, vm_params[clone].start_pa, vm_params[clone].start_pa + prev_size, vm_params[clone].end_pa - vm_params[clone].start_pa);
		memcpy((void *)(vm_params[clone].start_pa) + prev_size, (void *)(vm_params[clone].start_pa), vm_params[clone].end_pa - vm_params[clone].start_pa);
		set_net_phys_offset(idx, total_offset);
		dcclean_range((unsigned long)vm_params[clone].start_pa, vm_params[clone].end_pa - vm_params[clone].start_pa);

		vm_params[idx].start_pa = vm_params[clone].start_pa + prev_size;
		vm_params[idx].end_pa = vm_params[clone].end_pa + prev_size;
		vm_params[idx].start_va = vm_params[clone].start_va;
		vm_params[idx].end_va = vm_params[clone].end_va;
		vm_params[idx].pages = vm_params[clone].pages;

		guest_base += prev_size;
	} else {  // this is not a clone

		fdesc = open(elf,O_RDONLY);
		if (fdesc == -1) {
			error("\tCan't open file ", elf);
		}
		if (elf_get_ehdr(fdesc,&ehdr) < 0) {
			FAIL("\tInvalid ELF file: ", elf);
		}

		elf_get_specials(fdesc, vm_params[idx].specials, sizeof(vm_params[idx].specials)/sizeof(vm_params[idx].specials[0]), &ehdr);

		vm_params[idx].entry = (void *)ehdr.e_entry;
	

		for (i = 0; i < ehdr.e_phnum; i++) {
			if (elf_get_phdr(fdesc, i, &phdr, &ehdr) < 0) continue;
			if (phdr.p_memsz == 0) continue;
			if (phdr.p_type != PT_LOAD) continue;
			if (lseek(fdesc, phdr.p_offset, SEEK_SET) == -1) {
				error("\tCan't lseek() in ", elf);
			}
			if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;

			if (phdr.p_vaddr < start) {
				start = phdr.p_vaddr;
			}

			/* FIXME: Assuming first program header contains entry point and phys
				 offset is identical for all segments*/
			if (vm_params[idx].phys_offset == -1) { //unset
				vm_params[idx].phys_offset = phdr.p_vaddr - phdr.p_paddr;
			}
			/* FIXME: Assuming prog headers in sorted order.  Override with --load_offset if needed */
			if (vm_params[idx].load_offset == -1) {
				vm_params[idx].load_offset = guest_base - phdr.p_paddr;
			}
			total_offset = vm_params[idx].phys_offset + vm_params[idx].load_offset;

			if (vm_params[idx].offset_pages == -1) {
				vm_params[idx].offset_pages = (total_offset) >> (vm_params[idx].page_size * 2);
			}
			phdr.p_paddr += vm_params[idx].load_offset;

			if (set_fence_lo && (phdr.p_paddr < vm_params[idx].fence_lo)) {
				vm_params[idx].fence_lo = phdr.p_paddr;
			}

			if (!vm_params[idx].skip_load) {
				printf("\tload VA %08lx at %08lx\n", (unsigned long)phdr.p_vaddr, (unsigned long)phdr.p_paddr);
				bytes_read = 0;
				do {
					bytes_read += ret = read(fdesc,(char *)phdr.p_paddr + bytes_read, phdr.p_filesz - bytes_read);
				} while (ret > 0);
				if (ret == -1) {
					error("\tCan't read() in ", elf);
				}

				memset((char *)phdr.p_paddr+phdr.p_filesz, 0, phdr.p_memsz-phdr.p_filesz);
				/* Really, only need to clean out text sections */
				dcclean_range(phdr.p_paddr, phdr.p_memsz);
			}
		}
		close(fdesc);

		total_offset = vm_params[idx].phys_offset + vm_params[idx].load_offset;
		set_cmdline(idx, total_offset);
		set_net_phys_offset(idx, total_offset);
		if (vm_params[idx].use_dir_prefix) {
			set_var(idx, SPECIAL___use_dir_prefix__, 1, total_offset);
			set_string(idx, SPECIAL___dir_prefix__, vm_params[idx].dir_prefix, SIZE__dir_prefix__, total_offset);
		}
		if (vm_params[idx].use_file_suffix) {
			set_var(idx, SPECIAL___use_file_suffix__, 1, total_offset);
			set_string(idx, SPECIAL___file_suffix__, vm_params[idx].file_suffix, SIZE__file_suffix__, total_offset);
		}

		get_pmap(idx, total_offset);
		// if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;

		/* Adjust guest_base and fences */
		if (-1 == (end = vm_params[idx].specials[SPECIAL_end].addr)) {
			FAIL("\tCan't find end symbol", "");
		}
		printf("\tend 0x%08lx\n", end);

		vm_params[idx].start_pa = start + total_offset;
		vm_params[idx].end_pa = end + total_offset;
		dcclean_range((unsigned long)vm_params[idx].start_pa, vm_params[idx].end_pa - vm_params[idx].start_pa);

		heap_size = vm_params[idx].specials[SPECIAL_HEAP_SIZE].addr;
		if (0 == heap_size || -1 == heap_size) {
			if (0 == vm_params[idx].specials[SPECIAL_DEFAULT_HEAP_SIZE].addr
					|| -1 == vm_params[idx].specials[SPECIAL_DEFAULT_HEAP_SIZE].addr) {
				heap_size = GUESS_HEAP_SIZE;
				printf("\t** warning: heap size unknown, guessing 0x%08lx\n", heap_size);
			} else {
				heap_size = vm_params[idx].specials[SPECIAL_DEFAULT_HEAP_SIZE].addr;
				printf("\theap_size 0x%08lx (DEFAULT_HEAP_SIZE)\n", heap_size);
			}
		} else {
			printf("\theap_size 0x%08lx\n", heap_size);
		}

		stack_size = vm_params[idx].specials[SPECIAL_STACK_SIZE].addr;
		if (0 == stack_size || -1 == stack_size) {
			if (0 == vm_params[idx].specials[SPECIAL_DEFAULT_STACK_SIZE].addr
					|| -1 == vm_params[idx].specials[SPECIAL_DEFAULT_STACK_SIZE].addr) {
				stack_size = GUESS_STACK_SIZE;
				printf("\t** warning: stack size unknown, guessing 0x%08lx\n", stack_size);
			} else {
				stack_size = vm_params[idx].specials[SPECIAL_DEFAULT_STACK_SIZE].addr;
				printf("\tstack_size 0x%08lx (DEFAULT_STACK_SIZE)\n", stack_size);
			}
		} else {
			printf("\tstack_size 0x%08lx\n", stack_size);
		}

		end += heap_size + stack_size;
		vm_params[idx].stack = (void *)(end & -32);  // should be close to where crt0 puts the stack

		end = H2_ALIGN_UP(end, one_page);
		vm_params[idx].start_va = start;
		vm_params[idx].end_va = end;

		total_size = H2_ALIGN_UP((end - start), one_page);
		vm_params[idx].pages = total_size >> page_shift;

		printf("\ttotal_size 0x%08lx\n", total_size);
		guest_base += total_size;

		if (set_fence_lo) {
			vm_params[idx].fence_lo &= HI_MASK(one_page);
		}
		if (set_fence_hi) {
			if (tight_fence_hi) {
				vm_params[idx].fence_hi = vm_params[idx].fence_lo + total_size - one_page;
			} else {
				vm_params[idx].fence_hi = FENCE_HI_MAX & HI_MASK(one_page);
			}
		}
	}  // else not a clone

	clade_setup(idx, total_offset);

	printf("\tentry 0x%08lx\n", (unsigned long)vm_params[idx].entry);
	printf("\tphys_offset 0x%08lx\n", vm_params[idx].phys_offset);
	printf("\tload_offset 0x%08lx\n", vm_params[idx].load_offset);
	printf("\toffset_pages 0x%lx\n", vm_params[idx].offset_pages);

	if (vm_params[idx].pmap_added) {
		printf("\tguest translations added\n");
	} else {
		printf("\tfence_lo 0x%08x\n", vm_params[idx].fence_lo);
		printf("\tfence_hi 0x%08x\n", vm_params[idx].fence_hi);
	}
}

void config_vm(unsigned int idx) {

	unsigned long vm;

	H2K_offset_t base;
	int trans;
	

	printf("\n");
	printf("Config VM index %d\n", idx);
	printf("\tVirtual CPUs %d\n", vm_params[idx].num_vcpus);
	printf("\tShared interrupts  %d\n", vm_params[idx].num_shared_ints);
	printf("\tPriority  %d\n", vm_params[idx].startprio);

	vm = h2_config_vmblock_init(0, SET_CPUS_INTS, CONFIG_CPUS(vm_params[idx].use_ext, vm_params[idx].num_vcpus), vm_params[idx].num_shared_ints);

	base.size = vm_params[idx].page_size;
	base.cccc = vm_params[idx].cccc;
	base.xwru = vm_params[idx].xwru;
	base.pages = vm_params[idx].offset_pages;

	if (-1 == vm_params[idx].trans_type) {  // not set on cmdline
		if (NULL != vm_params[idx].pmap) {  // has __guest_pmap__
			trans = vm_params[idx].pmap->type;
			base.raw = vm_params[idx].pmap->base.raw;
		} else {  // default
			trans = H2K_ASID_TRANS_TYPE_OFFSET;
		}
	} else {  // translation type forced; better only be offset for now
		if (vm_params[idx].trans_type != H2K_ASID_TRANS_TYPE_OFFSET) {
			FAIL("\tAre you really going to type page tables on the command line?\n", "");
		}
		trans = vm_params[idx].trans_type;
	}

	if (h2_config_vmblock_init(vm, SET_PMAP_TYPE, (unsigned int)base.raw, trans) != vm) {
		FAIL("\tSET_PMAP_TYPE", "");
	}

	if (trans == H2K_ASID_TRANS_TYPE_OFFSET) {
		if (h2_config_vmblock_init(vm, SET_FENCES, vm_params[idx].fence_lo, vm_params[idx].fence_hi) != vm) {
			FAIL("\tSET_FENCES", "");
		}
	}

	if (h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, vm_params[idx].bestprio, vm_params[idx].trapmask) != vm) {
		FAIL("\tSET_PRIO_TRAPMASK", "");
	}

#if 0
	do {
	FIXME: in the future, we can map physical interrupts to guest with a command line option
			This was confusing things like the virtual timer interrupt.
			(Using if 0 here just to annoy Bryan.)
			/* set up interrupts */
			int i;	/* can move up to top and strike this block after reenabling maybe */
		for (i = 0; i < vm_params[idx].num_shared_ints + PERCPU_INTERRUPTS; i++) {
			if (h2_config_vmblock_init(vm, MAP_PHYS_INTR, i, CONFIG_PHYSINT_CPUID(i, vm_params[idx].num_vcpus - 1)) != vm) {
				FAIL("\tMAP_PHYS_INTR", "");
			}
		}
	} while (0);
#endif

	vm_params[idx].id = vm;
	printf("\tVM ID %lu\n", vm);
}

void boot_vm(unsigned int idx) {

	unsigned int regval;

	printf("\n");
	printf("Boot VM index %d, ID %d\n", idx, vm_params[idx].id);

	if (~0L != vm_params[idx].ccr) {
		regval = H2K_get_ccr();
		printf("\told value for ccr: 0x%08x\n",regval);
		H2K_set_ccr(vm_params[idx].ccr);
		regval = H2K_get_ccr();
		printf("\tnew value for ccr: 0x%08x\n",regval);
	}

	if (-1 == h2_vmboot(vm_params[idx].entry, vm_params[idx].stack, vm_params[idx].arg, vm_params[idx].startprio, vm_params[idx].id) ) {
		FAIL("\tfailed to boot vm\n", "");
	}
}

void run(unsigned int idx) {
	unsigned int i;
	unsigned int status, done;
	int cpus;
	unsigned int vm;
	unsigned long long int *res;
	unsigned int sigval;
	unsigned int hthreads_mask;

	for (i = 0 ; i <= idx; i++) {
		load_vm(i);
	}
	for (i = 0 ; i <= idx; i++) {
		config_vm(i);
	}

	/* Stats Reset */
	asm volatile (" r0 = #0x48 ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory");

	for (i = 0 ; i <= idx; i++) {
		boot_vm(i);
	}

	/* Wait for all VMs to stop or error */
	printf("\n");
	printf("booter: Waiting for interrupts\n");

	do {
		if (sample_usecs) {
			h2_vmtrap_timerop(H2K_TIMER_TRAP_DELTA_TIMEOUT, 1000 * sample_usecs);
		}
		sigval = h2_anysignal_wait(&wake_sig, WAKE_CHILD | WAKE_TIMER);

		if (sigval & WAKE_TIMER) {
			h2_anysignal_clear(&wake_sig, WAKE_TIMER);
			hthreads_mask = h2_prof_sample(&res);

			for (i = 0; i < MAX_HTHREADS; i++) {
				if (hthreads_mask & (1 << i)) {
					printf("Sample -- hthread: %d  ID: 0x%08x  ELR: 0x%08x\n", i, (unsigned int)(res[i] >> 32), (unsigned int)(res[i] & 0xffffffff));
				}
			}
			if (hthreads_mask) {
				h2_free(res);
			}
		}

		if (sigval & WAKE_CHILD) {
			h2_anysignal_clear(&wake_sig, WAKE_CHILD);

			/* How's everyone doing? */
			done = 1;
			for (i = 0; i <= idx; i++) {
				vm = vm_params[i].id;
				if (~0 == vm) {  // skip
					continue;
				}
				status = h2_vmstatus(VMOP_STATUS_STATUS, vm);
				printf("VM %d status 0x%x\n", vm, status);
				cpus = h2_vmstatus(VMOP_STATUS_CPUS, vm);
				printf("VM %d Live CPUs: %d\n", vm, cpus);

				if (0 == cpus) {  // no more cpus running
					if (status != vm_params[i].expect_status && vm_params[i].error_exit) {
						FAIL("\tUnexpected exit status.", "");
					}
					if (--vm_params[i].boots) {  // reboot
						done = 0;
						load_vm(i);
						boot_vm(i);
					} else {  // all done with this VM
						vm_params[i].id = ~0;  // mark non-existent
						h2_vmfree(vm);
					}
				} else {
					done = 0; // someone's not done
				}
				if (vm_params[i].error_exit && H2_THREAD_FATAL_ERROR == status) {
					exit(1);
				}
			}
		}
	} while (!done);

	H2K_set_syscfg(h2_info(INFO_SYSCFG) & ~SYSCFG_CLADEN);  // disable clade
	pd_num = 0;  // reset
	h2_galloc_reset(&tcm_alloc, 0);
}

void die_usage()
{
	usage();
	exit(1);
}

void print_infos() {

	printf("H2/core info:\n");
	printf("\tBuild ID: 0x%08x\n", h2_info(INFO_BUILD_ID));
	printf("\tGuest PC sampling available: ");
	printf((boot_flags.boot_have_sample ? "true\n" : "false\n"));
	printf("\tHVX:\n");
	printf("\t\tPresent: %s\n", (boot_flags.boot_have_hvx ? "true" : "false"));
	if (boot_flags.boot_have_hvx) {
		printf("\t\tNative vector length: %d\n", h2_info(INFO_HVX_VLENGTH));
		printf("\t\tContexts (when v2x == 0): %d\n", h2_info(INFO_HVX_CONTEXTS));
		printf("\t\tCan context-switch in kernel: %s\n", (boot_flags.boot_ext_ok ? "true" : "false"));
	}
	printf("\tKernel physical address: 0x%08x\n", h2_info(INFO_PHYSADDR));
	printf("\tKernel page size: %dK\n", h2_info(INFO_H2K_PGSIZE) / 1024);
	printf("\tNumber of kernel pages: %d\n", h2_info(INFO_H2K_NPAGES));
	printf("\tH2 kernel in TCM: ");
	printf((boot_flags.boot_use_tcm ? "true\n" : "false\n"));
	printf("\tTCM (adjusted) base: 0x%08x\n", tcm_base);
	printf("\tTCM (remaining) size: %dK\n", tcm_size / 1024);
	printf("\tL2 array size: %dK\n", h2_info(INFO_L2MEM_SIZE) / 1024);
	printf("\tL2 cache size: %dK\n", h2_info(INFO_L2TAG_SIZE) / 1024);
	printf("\tL2 register base: 0x%08x\n", h2_info(INFO_L2CFG_BASE));
	printf("\tCLADE register base: 0x%08x\n", clade_base);

	printf("\tTLB entries: %d\n", h2_info(INFO_TLB_SIZE));
	printf("\tReplaceable TLB entries: %d\n", h2_info(INFO_TLB_FREE));
	printf("\tSTLB:\n");
	printf("\t\tEnabled: ");
	if (stlb_info.stlb_enabled) {
		printf("true\n");
		printf("\t\tSets per ASID: %d\n", 1 << stlb_info.stlb_max_sets_log2);
		printf("\t\tWays: %d\n", stlb_info.stlb_max_ways);
		printf("\t\tSize: %d\n", stlb_info.stlb_size);
		printf("\t\tEntries: %dK\n", ((1 << stlb_info.stlb_max_sets_log2) * stlb_info.stlb_max_ways * stlb_info.stlb_size) / 1024);
	} else {
		printf("false\n");
	}
	printf("\tsyscfg: 0x%08x\n", h2_info(INFO_SYSCFG));
	printf("\trev: 0x%08x\n", h2_info(INFO_REV));
	printf("\tSubsystem base: 0x%08x\n", h2_info(INFO_SSBASE));
	printf("\tL2VIC physical base: 0x%08x\n", h2_info(INFO_L2VIC_BASE));
	printf("\tTimer physical base: 0x%08x\n", h2_info(INFO_TIMER_BASE));
	printf("\tTimer interrupt: %d\n", h2_info(INFO_TIMER_INT));
	printf("\tRunning HW threads mask: 0x%08x\n", h2_info(INFO_HTHREADS));
}

void kernel_setup() {

	if (use_stlb) {
		if (h2_config_stlb_alloc() < 0) {
			FAIL("STLB alloc", "");
		}
	}
#if HAVE_EXTENSIONS
	if (ext_power && boot_flags.boot_have_hvx) {
		if (h2_hwconfig_extpower(1) < 0) {
			FAIL("extpower", "");
		}
	}
#endif
}

void set_l2_reg(unsigned int offset, unsigned int val) {

	unsigned int old, ret;

	printf("Set L2 reg at offset 0x%08x:\n", offset);

	old = h2_hwconfig_l2_get_reg(offset);
	printf("\tOld value:  0x%08x\n", old);

	if (val != -1) {
		ret = h2_hwconfig_l2_set_reg(offset, val);

		if (ret != old) {
			FAIL("set_l2_reg mismatch.", "");
		}

		printf("\tNew value:  0x%08x\n", val);
	}
}

/* Need to clean when clearing L2WB */
void set_l2wb (unsigned int val) {
	unsigned int old = h2_info(INFO_SYSCFG);

	// Leave L2CFG unchanged
	if (h2_hwconfig_l2cache_size((old & SYSCFG_L2CFG) >> SYSCFG_L2CFG_BITS, val) == -1) {
		FAIL("HWCONFIG_L2CACHE", "");
	}
}

typedef struct {
	const char *name;
	int pos;
	int len;  // bits
	void (* handler)(unsigned int);
} syscfg_field;

syscfg_field syscfg[] = {
	{"BQ", SYSCFG_BQ_BIT, SYSCFG_BQ_LEN, H2K_set_syscfg},
	{"DMT", SYSCFG_DMT_BIT, SYSCFG_DMT_LEN, H2K_set_syscfg},
	{"CLADEN", SYSCFG_CLADEN_BIT, SYSCFG_CLADEN_LEN, H2K_set_syscfg},
	{"L2WB", SYSCFG_L2WB_BIT, SYSCFG_L2WB_LEN, set_l2wb},
	{NULL, 0, 0}
};

void set_syscfg_field(char *name, unsigned int val) {

	int i = 0;
	unsigned int old = h2_info(INFO_SYSCFG);
	unsigned int mask;

	while (syscfg[i].name != NULL) {
		if (strcasecmp(syscfg[i].name, name) == 0) {
			mask = (( 0x1 << syscfg[i].len) - 1);
			val = val & mask;
			val <<= syscfg[i].pos;
			mask <<= syscfg[i].pos;
			syscfg[i].handler((old & ~mask) | val);
			return;
		}
		i++;
	}
	printf("Unknown SYSCFG bit %s\n", name);
	FAIL("set_syscfg_field", "");
}

extern void bootvm_vectors();

void booter_isr(unsigned int gssr) {

	if ((gssr & 0xff) == H2K_VM_CHILDINT) {
		//		printf("Got child interrupt\n");
		h2_anysignal_set(&wake_sig, WAKE_CHILD);
		h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_VM_CHILDINT, 0);
	}
	if ((gssr & 0xff) == H2K_TIME_GUESTINT) {
		//		printf("Got timer interrupt\n");
		h2_anysignal_set(&wake_sig, WAKE_TIMER);
		h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0);
	}

}

unsigned int process_line(int argc, char **argv, unsigned int idx) {

	unsigned int vm_instances = 0;

	unsigned int regval;
	int finish = 0;

	while (argc) {

		/* Global options */

		if (0 == strcmp(argv[0],"--syscfg")) {
			if (argc < 2) die_usage();
			regval = h2_info(INFO_SYSCFG);
			printf("Old value for syscfg: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_syscfg(regval);
			regval = h2_info(INFO_SYSCFG);
			printf("New value for syscfg: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--livelock")) {
			if (argc < 2) die_usage();
			regval = h2_info(INFO_LIVELOCK);
			printf("Old value for livelock: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_livelock(regval);
			regval = h2_info(INFO_LIVELOCK);
			printf("New value for livelock: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--duck")) {
			if (argc < 2) die_usage();
			regval = H2K_get_duck();
			printf("Old value for duck: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_duck(regval);
			regval = H2K_get_duck();
			printf("New value for duck: 0x%08x\n",regval);
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

		} else if (0 == strcmp(argv[0], "--l1dp")) {
			if (argc < 2) die_usage();
			if (h2_hwconfig_partition(HWCONFIG_PARTITION_D, strtoul(argv[1],NULL,0)) == -1) {
				FAIL("HWCONFIG_PARTITION_D", "");
			}
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--l1ip")) {
			if (argc < 2) die_usage();
			if (h2_hwconfig_partition(HWCONFIG_PARTITION_I, strtoul(argv[1],NULL,0)) == -1) {
				FAIL("HWCONFIG_PARTITION_I", "");
			}
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--l2part")) {
			if (argc < 2) die_usage();
			if (h2_hwconfig_partition(HWCONFIG_PARTITION_L2, strtoul(argv[1],NULL,0)) == -1) {
				FAIL("HWCONFIG_PARTITION_L2", "");
			}
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--l2cfg")) {
			if (argc < 2) die_usage();

			// Keep L2WB unchanged; use --syscfg_bit to modify that
			regval = h2_info(INFO_SYSCFG);
			if (h2_hwconfig_l2cache_size(strtoul(argv[1],NULL,0), (regval & SYSCFG_L2WB) >> SYSCFG_L2WB_BIT) == -1) {
				FAIL("HWCONFIG_L2CACHE", "");
			}
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--syscfg_bit")) {
			if (argc < 3) die_usage();
			set_syscfg_field(argv[1], strtoul(argv[2], NULL, 0));
			argc -= 3; argv += 3;
			continue;

		} else if (0 == strcmp(argv[0], "--l2_reg")) {
			if (argc < 3) die_usage();
			set_l2_reg(strtoul(argv[1], NULL, 0), strtoul(argv[2], NULL, 0));
			argc -= 3; argv += 3;
			continue;

#ifdef HAVE_EXTENSIONS
		} else if (0 == strcmp(argv[0], "--ext_power")) {
			if (argc < 2) die_usage();
			ext_power = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
#endif

		} else if (0 == strcmp(argv[0], "--use_stlb")) {
			if (argc < 2) die_usage();
			use_stlb = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--guest_base")) {
			if (argc < 2) die_usage();
			guest_base = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--sample")) {
			if (argc < 2) die_usage();
			if (!boot_flags.boot_have_sample) {
				FAIL("Guest PC sampling not supported by kernel", "");
			}
			sample_usecs = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--help")) {
			usage();
			exit(0);

			/* Per-VM options */

		} else if (0 == strcmp(argv[0],"--ccr")) {
			if (argc < 2) die_usage();
			vm_params[idx].ccr = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--num_vcpus")) {
			if (argc < 2) die_usage();
			vm_params[idx].num_vcpus = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

#ifdef HAVE_EXTENSIONS
		} else if (0 == strcmp(argv[0], "--use_ext")) {
			if (argc < 2) die_usage();
			vm_params[idx].use_ext = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
#endif

		} else if (0 == strcmp(argv[0], "--num_shared_ints")) {
			if (argc < 2) die_usage();
			vm_params[idx].num_shared_ints = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--page_size")) {
			if (argc < 2) die_usage();
			vm_params[idx].page_size = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--cccc")) {
			if (argc < 2) die_usage();
			vm_params[idx].cccc = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--offset_pages")) {
			if (argc < 2) die_usage();
			vm_params[idx].offset_pages = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--translation_type")) {
			if (argc < 2) die_usage();
			vm_params[idx].trans_type = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--fence_lo")) {
			if (argc < 2) die_usage();
			vm_params[idx].fence_lo = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--fence_hi")) {
			if (argc < 2) die_usage();
			vm_params[idx].fence_hi = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--load_offset")) {
			if (argc < 2) die_usage();
			vm_params[idx].load_offset = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--skip_load")) {
			if (argc < 2) die_usage();
			vm_params[idx].skip_load = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--bestprio")) {
			if (argc < 2) die_usage();
			vm_params[idx].bestprio = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--trapmask")) {
			if (argc < 2) die_usage();
			vm_params[idx].trapmask = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--stack")) {
			if (argc < 2) die_usage();
			vm_params[idx].stack = (void *)strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--arg")) {
			if (argc < 2) die_usage();
			vm_params[idx].arg = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--boots")) {
			if (argc < 2) die_usage();
			vm_params[idx].boots = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--expect_status")) {
			if (argc < 2) die_usage();
			vm_params[idx].expect_status = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--error_exit")) {
			if (argc < 2) die_usage();
			vm_params[idx].error_exit = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--startprio")) {
			if (argc < 2) die_usage();
			vm_params[idx].startprio = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--dir_prefix")) {
			if (argc < 2) die_usage();
			vm_params[idx].dir_prefix = argv[1];
			vm_params[idx].use_dir_prefix = 1;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--file_suffix")) {
			if (argc < 2) die_usage();
			vm_params[idx].file_suffix = argv[1];
			vm_params[idx].use_file_suffix = 1;
			argc -= 2; argv += 2;
			continue;

		} else {
			while (argc) {
				if (0 == strcmp(argv[0], "--new_vm")) {
					if (argc < 2) { 
						die_usage();
					}
					tight_fence_hi = 1;  // multiple VMs, need to keep fences tight

					if (finish) {  // this --new_vm is ending opts for a previous one
						clone_vm(idx, vm_instances);
						idx += vm_instances + 1;
						add_vm(idx);
					}
					vm_instances = strtoul(argv[1], NULL, 0) - 1;
					argc -= 2; argv += 2;
					break;  // to top of outer loop

				} else {  // executable and its options
					finish = 1;  // from now on --new_vm always finishes a previous one

					if (0 == argc) {
						die_usage();
					}
					if (0 == vm_params[idx].argc) {  // first arg
						vm_params[idx].argv = argv;
					}
					vm_params[idx].argc++;

					argc--;
					argv++;
				}
			}
		}
	}

	clone_vm(idx, vm_instances);
	idx += vm_instances;

	return idx;
}

size_t getline(char **lineptr, size_t *n, FILE *stream) {

	char *buf = NULL;
	char *p = buf;
	size_t size;
	int c;

	if (NULL == lineptr) {
		return -1;
	}
	if (NULL == stream) {
		return -1;
	}
	if (NULL == n) {
		return -1;
	}
	buf = *lineptr;
	size = *n;

	c = fgetc(stream);
	if (c == EOF) {
		return -1;
	}
	if (NULL == buf) {
		buf = malloc(128);
		if (NULL == buf) {
			return -1;
		}
		size = 128;
	}
	p = buf;
	while(c != EOF) {
		if ((p - buf) > (size - 1)) {
			size = size + 128;
			buf = realloc(buf, size);
			if (NULL == buf) {
				return -1;
			}
		}
		*p++ = c;
		if (c == '\n') {
			break;
		}
		c = fgetc(stream);
	}

	*p++ = '\0';
	*lineptr = buf;
	*n = size;

	return p - buf - 1;
}

extern void pthread_init();

int main(int argc, char **argv)
{
	unsigned int kerror;

	char **cur_argv = NULL;
	int cur_argc = 0;
	FILE *fp;
	char *line = NULL;
	size_t line_size;
	char *arg_ptr;
	char *p;

	int idx;

	//Remove booter from cmdline
	pthread_init();
	strncpy(errstr, argv[0], ERRSTR_LEN - 1);
	argc--;
	argv++;

	if (argc < 1) {
		usage();
		return 1;
	}

	// check for kernel boot errors
	kerror = h2_info(INFO_ERROR);
	if (kerror != KERROR_NONE) {
		printf("\n");
		printf("Kernel error: %s\n\n", kerror_msg[kerror]);
		print_infos();
		return 1;
	}

	boot_flags.raw = h2_info(INFO_BOOT_FLAGS);
	stlb_info.raw = h2_info(INFO_STLB);

	tcm_base = (unsigned int)h2_info(INFO_TCM_BASE);
	tcm_size = h2_info(INFO_TCM_SIZE);
	clade_base = h2_info(INFO_CLADE_BASE);
	h2_galloc_init(&tcm_alloc, (unsigned int)tcm_base, (unsigned int)tcm_size, NULL);

	kernel_setup();
	print_infos();
	h2_vmtrap_setvec(bootvm_vectors);

	h2_anysignal_init(&wake_sig);

	if (h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_VM_CHILDINT, 0) < 0) {
		FAIL("H2K_INTOP_GLOBEN, H2K_VM_CHILDINT", "");
	}
	if (h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0) < 0) {
		FAIL("H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT", "");
	}
	h2_vmtrap_setie(1);

	/* Each file specifies a set of VMs to run concurrently */
	while (argc) {
		if (0 == strcmp(argv[0],"--file")) {
			if (argc < 2) die_usage();
			if (NULL == (fp = fopen(argv[1], "r"))) {
				error("Can't open ", argv[1]);
			}
			argc -= 2;
			argv += 2;

			idx = -1;

			/* Run VMs from each file concurrently */
			while (-1 != getline(&line, &line_size, fp)) {
				if ((p = strchr(line, '\n'))) {
					*p = '\0';  // chop \n
				}
				arg_ptr = strtok(line, " ");
				while (arg_ptr) {
					if ('#' == *arg_ptr) {  // start of comment
						break;
					}
					cur_argv = realloc(cur_argv, sizeof(char *) * ++cur_argc);
					if (NULL == cur_argv) {
						error("realloc cur_argv", NULL);
					}
					cur_argv[cur_argc - 1] = arg_ptr;
					arg_ptr = strtok(NULL, " ");
				}
				if (!cur_argc) {  // empty line
					goto next;
				}
					
				if (++idx) {
					tight_fence_hi = 1;
				}
				add_vm(idx);
				idx = process_line(cur_argc, cur_argv, idx);

			next:
				/* NULL causes new malloc */
				line = NULL;
				cur_argv = NULL;
				cur_argc = 0;
			}
			fclose(fp);

			run(idx);
			free(vm_params);
			vm_params = NULL;  // malloc anew if more --files
			tight_fence_hi = 0;
			guest_base = H2K_GUEST_START;

			h2_anysignal_init(&wake_sig);  // could be leftovers

		} else {  // not reading from file
			idx = 0;
			add_vm(idx);
			idx = process_line(argc, argv, idx);
			run(idx);
			return 0;
		}
	}
	return 0;
}

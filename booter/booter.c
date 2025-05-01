/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//#include <max.h>
#include <h2.h>
#include <pthread.h>
#include <stdio.h>
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
#include <h2_coproc.h>

#include <fcntl.h>
#include <unistd.h>

#include "elf.h"
#include "../kernel/include/max.h"
#include "../kernel/include/hw.h"
#include <syscall_defs.h>

#define VM_BEST_PRIO 0

#define BUFSIZE 256

/* Misc */
char *trans_name[] = {
	"linear",
	"table",
	"invalid",
	"offset"
};

char *pagesize_name[] = {
	"4KB",
	"16KB",
	"64KB",
	"256KB",
	"1MB",
	"4MB",
	"16MB"
};

#define FOREACH_sym(GEN)												\
	GEN(__guest_pmap__)														\
	GEN(__boot_cmdline__)													\
	GEN(__boot_net_phys_offset__)									\
	GEN(__use_dir_prefix__)												\
	GEN(__use_file_suffix__)											\
	GEN(__dir_prefix__)														\
	GEN(__file_suffix__)													\
	GEN(__h2_pmu_evtcfg__)												\
	GEN(__h2_pmu_evtcfg1__)												\
	GEN(__h2_pmu_cfg__)   												\
	GEN(__h2_gpio_toggle__)												\
	GEN(__sys_write_mode__)												\
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

#define DMA_CTRL_SET_VAL 0x2e3
#define DMA_CTRL_SET_INDEX 2

#define PMU_COUNTER_NONE 0
#define PMU_COUNTER0_OVERFLOW 1
#define PMU_COUNTER2_OVERFLOW 2
#define PMU_COUNTER4_OVERFLOW 5
#define PMU_COUNTER6_OVERFLOW 6
#define PMU_FILE "pmu_stats_booter.txt"

#define PMU_FIRST_EVENT 0
#if ARCHV >= 68
#define PMU_LAST_EVENT 1023
#define PMU_LAST_EVENT_STR "1023"
#else
#define PMU_LAST_EVENT 511
#define PMU_LAST_EVENT_STR "511"
#endif
#define PMU_BUFSIZE (PMU_LAST_EVENT * 32)
/* Globals */
#ifdef HAVE_EXTENSIONS
unsigned int ext_power = 1;
#endif
unsigned int silent = 0;
unsigned int cycles = 0;
unsigned int use_stlb = 0;
unsigned int tight_fence_hi = 1;
unsigned long guest_base;
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
int hwt_mask = -1;
int hwt_num = -1;
int ecc_enable = -1;
int set_pmu_evtcfg = 0;
int set_pmu_evtcfg1 = 0;
int set_pmu_cfg = 0;
unsigned int pmu_evtcfg;
unsigned int pmu_evtcfg1;
unsigned int pmu_cfg;
int pmu_dump = 0;
char *pmu_file = PMU_FILE;
FILE *pmu_fp;
char pmu_buf[PMU_BUFSIZE];
int pmu_idx = 0;
int gpio_toggle = 0;
int set_dmactrl = 0;
int dmactrl;
#ifdef CLUSTER_SCHED
int cluster_sched = 1;
#endif
unsigned int getl2reg = 0;
unsigned int *getl2reg_offsets;
#ifdef MULTICORE
unsigned int core_id = -1;
unsigned int core_count = -1;
#endif

extern long __boot_net_phys_offset__;

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
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	unsigned int vwctrl;
#endif

	/* Translation options from cmdline */
	unsigned int page_size;
	unsigned int cccc;
	unsigned int xwru;
	unsigned long offset_pages;
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
	unsigned int reboot_reload;
	unsigned int stash;
	unsigned long long int stash_addr;

	/* exit on error */
	unsigned int error_exit;

	/* PMU */
	int pmu_sweep;
	int pmu_first_event;
	int pmu_last_event;
	int pmu_overflow;
	sys_write_mode sys_write_mode;
	int va_angel;

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

void FAIL(const char *str1, const char *str2)
{
	BOOTER_PRINTF("FAIL %s %s\n", str1, str2);
	exit(1);
}

void pcycles(void) {
	unsigned long long int cyc;

	if (!cycles) return;
	
	asm volatile ( " %0 = c15:14 // upcycle \n" : "=r"(cyc));
	BOOTER_PRINTF("\nPCYCLES %llu\n", cyc);
}

void usage()
{
	BOOTER_PRINTF("Usage:\n");
	BOOTER_PRINTF("  booter [options] <executable> <args>\n");
	BOOTER_PRINTF("  booter [options] <executable> <args> [ --new_vm <instances> [vm options] <executable> <args> ...]\n");
	BOOTER_PRINTF("  booter [global options] --new_vm <int> [vm options] <executable> <args> [--new_vm <int> [vm options] <executable> <args> ...]\n\tBoot new <instances> of guest VMs with given options.\n");
	BOOTER_PRINTF("  booter --file <path> [--file <path> ...]\n\tOptions from <path>, one guest VM per line.");
	BOOTER_PRINTF("\n");
	BOOTER_PRINTF("Global options:\n");
#ifdef MULTICORE
	BOOTER_PRINTF("  --core <int>\n\tIf current core ID is not <int>, ignore all following options until the next --core.\n");
#endif
	BOOTER_PRINTF("  --turkey <int>\n\tSet the turkey bits.\n");
	BOOTER_PRINTF("  --duck <int>\n\tSet the duck bits.\n");
	BOOTER_PRINTF("  --chicken <int>\n\tSet the chicken bits.\n");
	BOOTER_PRINTF("  --rgdr <int>\n\tSet rgdr.\n");
	BOOTER_PRINTF("  --syscfg <int>\n\tSet syscfg.\n");
	BOOTER_PRINTF("  --livelock <int>\n\tSet livelock.\n");
	BOOTER_PRINTF("  --syscfg_bit <name> <int>\n\tSet syscfg bit(s) not covered by other options.\n");
	BOOTER_PRINTF("  --dma_ctrl <int>\n\tSet user DMA control register (DM2).\n");
	BOOTER_PRINTF("  --l1dp [ 0 == shared, 1 == 1/2 main, 2 == 3/4 main ]\n\tSet L1 data cache partitioning (ARCHV <= 5).\n");
	BOOTER_PRINTF("  --l1ip [ 0 == shared, 1 == 1/2 main, 2 == 3/4 main ]\n\tSet L1 instruction cache partitioning (ARCHV <= 5).\n");
	BOOTER_PRINTF("  --l2part [ 0 == shared, 1 == 1/2 main, 2 == 3/4 main, 3 == 7/8 main ]\n\tSet L2 cache partitioning.\n");
	BOOTER_PRINTF("  --l2cfg <int>\n\tSet L2 cache tag size bits.\n");
	BOOTER_PRINTF("  --l2_reg <offset int> <int>\n\tSet L2 config register.\n");
	BOOTER_PRINTF("  --get_l2_reg <offset int>\n\tPrint value of L2 config register.\n");
	BOOTER_PRINTF("  --stride_prefetch_reg <offset int> <int>\n\tSet stride prefetcher register.\n");
	BOOTER_PRINTF("  --dvlm_reg <offset int> <int>\n\tSet DPM voltlimit register.\n");
#ifdef HAVE_EXTENSIONS
	BOOTER_PRINTF("  --ext_power (0|1)\n\tPower on/off coprocessors.  Default 1.\n");
	BOOTER_PRINTF("  --hmx_poweron_addr <addr>\n\tSet HMX RSC sequence power-on start address.\n");
	BOOTER_PRINTF("  --hmx_poweroff_addr <addr>\n\tSet HMX RSC sequence power-off start address.\n");
#endif
	BOOTER_PRINTF("  --use_stlb (0|1)\n\tTurn on STLB.  Default 0.\n");
	BOOTER_PRINTF("  --guest_base <int>\n\tStart of guest physical memory. Default 0x%08x.\n", H2K_GUEST_START);
	BOOTER_PRINTF("  --sample <int>\n\tSet guest PC sample interval in usecs. Default 0 (disabled).\n");
	BOOTER_PRINTF("  --hwt_mask <int>\n\tMask of hardware threads to start. Default -1 (all).\n");
	BOOTER_PRINTF("  --hwt_num <int>\n\tNumber of hardware threads to start. Default -1 (all).\n");
	BOOTER_PRINTF("  --ecc_enable <int>\n\tMask of memories to enable/disable ECC.  Default 0.\n");
	BOOTER_PRINTF("  --pmu_evtcfg <int>\n\tSet PMU event config register 0.\n");
	BOOTER_PRINTF("  --pmu_evtcfg1 <int>\n\tSet PMU event config register 1.\n");
	BOOTER_PRINTF("  --pmu_cfg <int>\n\tSet PMU config register.\n");
	BOOTER_PRINTF("  --pmu_dump (0|1)\n\tDump PMU counters.  Default 0.\n");
	BOOTER_PRINTF("  --pmu_file <string>\n\tPMU output file name. Default " PMU_FILE ".\n");
	BOOTER_PRINTF("  --gpio_toggle (0|1)\n\tToggle power-measurement GPIO on PMU enable/disable.  Default 0.\n");
	BOOTER_PRINTF("  --gpio_addr <addr>\n\tSet power-measurement GPIO physical address.\n");
	BOOTER_PRINTF("  --cycles\n\tPrint pcycles during boot.\n");
#ifdef MULTICORE
	BOOTER_PRINTF("  --quiet <core bitmap>\n\tSuppress output from cores.\n");
#else
	BOOTER_PRINTF("  --quiet\n\tSuppress output.\n");
#endif
	BOOTER_PRINTF("\n");
	BOOTER_PRINTF("VM options:\n");
	BOOTER_PRINTF("  --ccr <int>\n\tSet ccr.\n");
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	BOOTER_PRINTF("  --vwctrl <int>\n\tSet vwctrl.\n");
#endif
	BOOTER_PRINTF("  --num_vcpus <int>\n\tMax number of virtual CPUs. Default 32.\n");
#ifdef HAVE_EXTENSIONS
	BOOTER_PRINTF("  --use_ext (0|1)\n\tSupport extended contexts.  Default 0.\n");
#endif
	BOOTER_PRINTF("  --num_shared_ints <int>\n\tNumber of shared interrupts.  Default 0.\n");
	BOOTER_PRINTF("  --page_size [ 0 == 4K, 1 == 16K, 2 == 64K, 3 == 256K, 4 == 1M, 5 == 4M, 6 == 16M ]\n\tEncoded page size for guest->phys offset map.  Default 6 (16M).\n");
	BOOTER_PRINTF("  --cccc <int>\n\tCache bits for guest->phys offset map.  Default L1WB_L2C (0xa == L1WB_L2CWB_AUX).\n");
	BOOTER_PRINTF("  --offset_pages <int>\n\tOffset (in number of pages) for guest->phys offset map.  Default matches load_offset, or 0.\n");
	BOOTER_PRINTF("  --translation_type [ %d == OFFSET ]\n\tTranslation type for guest->phys map.  Default OFFSET (only OFFSET works from cmdline right now.  Used to override guest_pmap).\n", H2K_ASID_TRANS_TYPE_OFFSET);
	BOOTER_PRINTF("  --fence_lo <int>\n\tLowest physical page accessible by guest VM.  Must be page_size-aligned.  Default lowest mapped physical page.\n");
	BOOTER_PRINTF("  --fence_hi <int>\n\tHighest physical page accessible by guest VM.  Must be page_size-aligned.  Default (end - fence_lo) + heap size + stack size.\n");
	BOOTER_PRINTF("  --load_offset <int>\n\tOffset for loading ELF image.  Default (guest_base - <first_program_header_addr>).\n");
	BOOTER_PRINTF("  --skip_load (0|1)\n\tSkip program loading (e.g. if loaded by simulator with --extra_elf).  Default 0.\n");
	BOOTER_PRINTF("  --bestprio <int>\n\tBest allowed priority for a virtual CPU.  Default 0.\n");
	BOOTER_PRINTF("  --trapmask <int>\n\tBitmask of allowed trap0 numbers.  Default 0xffffffff (all allowed).\n");
	BOOTER_PRINTF("  --stack <int>\n\tStack pointer VA for first virtual CPU.  Default 0xfeffff8.\n");
	BOOTER_PRINTF("  --arg <int>\n\tInitial argument (R0) for first virtual CPU.  Default 0.\n");
	BOOTER_PRINTF("  --boots <int>\n\tNumber of times to boot the VM, if exiting with expected status.  Default 1.\n");
	BOOTER_PRINTF("  --expect_status <int>\n\tReboot-request status value. The last virtual CPU is expected to vmstop with this status, in which case the VM is started again if the requested number of boots has not been reached.  Default 0.\n");
	BOOTER_PRINTF("  --reboot_reload (0|1)\n\tReload program when rebooting VM.  Default 0.\n");
	BOOTER_PRINTF("  --stash (0|1)\n\tStash a copy of VM load image for reboot.  Default 0.\n");
	BOOTER_PRINTF("  --stash_addr <int>\n\tStart address for stash. Default determined by booter.\n");
	BOOTER_PRINTF("  --error_exit (0|1)\n\tExit when a virtual CPU stops on fatal error.  Default 1.\n");
	BOOTER_PRINTF("  --pmu_sweep (0|1)\n\tCollect PMU counters over multiple runs.  Default 0.\n");
	BOOTER_PRINTF("  --pmu_first_event <int>\n\tFirst PMU event for sweep.  Default 0.\n");
	BOOTER_PRINTF("  --pmu_last_event <int>\n\tLast PMU event for sweep.  Default " PMU_LAST_EVENT_STR ".\n");
	BOOTER_PRINTF("  --pmu_overflow (0|1)\n\tUse 64-bit PMU counters in sweep.  Default 0.\n");
	BOOTER_PRINTF("  --sys_write_mode [ 0 == normal, 1 == suppress stdout, 2 == allow only stdout, 3 == suppress all ]\n\tMode for sys_write().  Default 0.\n");
	BOOTER_PRINTF("  --va_angel (0|1)\n\tUse virtual addresses in angel calls.  Default 0.\n");
	BOOTER_PRINTF("  --startprio <int>\n\tInitial priority of first virtual CPU.  Default 0.\n");
	BOOTER_PRINTF("  --dir_prefix <string>\n\tPrepend <string> to relative paths when opening files. Default null string.\n");
	BOOTER_PRINTF("  --file_suffix <string>\n\tAppend <string> to file names when opening files write-only. Default null string.\n");
#ifdef CLUSTER_SCHED
	BOOTER_PRINTF("  --cluster_sched (0|1)\n\tEnable cluster-restricted scheduling for HVX.  Default 1.\n");
#endif
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
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	vm_params[idx].vwctrl = ~0L;
#endif

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
	vm_params[idx].reboot_reload = 0;
	vm_params[idx].stash = 0;
	vm_params[idx].stash_addr = 0;
	vm_params[idx].error_exit = 1;

	vm_params[idx].pmu_sweep = 0;
	vm_params[idx].pmu_first_event = PMU_FIRST_EVENT;
	vm_params[idx].pmu_last_event = PMU_LAST_EVENT;
	vm_params[idx].pmu_overflow = 0;
	vm_params[idx].sys_write_mode = H2_SYS_WRITE_MODE_NORMAL;
	vm_params[idx].va_angel = 0;

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
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
		vm_params[idx + num].vwctrl = ~0L;
#endif
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
		vm_params[idx + num].reboot_reload = vm_params[idx].reboot_reload;
		vm_params[idx + num].stash = vm_params[idx].stash;
		vm_params[idx + num].stash_addr = vm_params[idx].stash_addr;
		vm_params[idx + num].error_exit = vm_params[idx].error_exit;

		/* FIXME: Maybe clones shouldn't set the PMU, but they do need to run each time */
		vm_params[idx + num].pmu_sweep = vm_params[idx].pmu_sweep;
		vm_params[idx + num].pmu_first_event = vm_params[idx].pmu_first_event;
		vm_params[idx + num].pmu_last_event = vm_params[idx].pmu_last_event;
		vm_params[idx + num].pmu_overflow = vm_params[idx].pmu_overflow;
		vm_params[idx + num].sys_write_mode = vm_params[idx].sys_write_mode;
		vm_params[idx + num].va_angel = vm_params[idx].va_angel;

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
		BOOTER_PRINTF("\t__guest_pmap__ not found.\n");
	} else {
		BOOTER_PRINTF("\t__guest_pmap__ found @ 0x%08lx\n",addr);

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
		BOOTER_PRINTF("\t__boot_cmdline__ not found.\n");
		return;
	} else {
		BOOTER_PRINTF("\t__boot_cmdline__ found @ 0x%08x\n", (unsigned int)addr);
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

	BOOTER_PRINTF("\tcmdline at 0x%08x set to <<%s>>\n", (unsigned int)dst, dst);
}

void set_string(unsigned int idx, int sym, char *string, int maxlen, long offset) {

	char *dst;
	unsigned long addr;

	if (NULL == string) return;

	if (strlen(string) >= maxlen - 1) {
		FAIL(vm_params[idx].specials[sym].name, ": string too long");
	}

	if ((addr = vm_params[idx].specials[sym].addr) == -1) {
		BOOTER_PRINTF("\t%s not found.\n", vm_params[idx].specials[sym].name);
		return;
	} else {
		BOOTER_PRINTF("\t%s found @ 0x%08x\n", vm_params[idx].specials[sym].name, (unsigned int)addr);
	}

	dst = (char *)(addr + offset);
	strcpy(dst, string);

	BOOTER_PRINTF("\t%s at 0x%08x set to <<%s>>\n", vm_params[idx].specials[sym].name, (unsigned int)dst, dst);
}

void set_var(unsigned int idx, int sym, int val, long offset) {

	int *dst;
	unsigned long addr;

	if ((addr = vm_params[idx].specials[sym].addr) == -1) {
		BOOTER_PRINTF("\t%s not found.\n", vm_params[idx].specials[sym].name);
		return;
	} else {
		BOOTER_PRINTF("\t%s found @ 0x%08x\n", vm_params[idx].specials[sym].name, (unsigned int)addr);
	}

	dst = (int *)(addr + offset);
	*dst = val;

	BOOTER_PRINTF("\t%s at 0x%08x set to <<0x%08x>>\n", vm_params[idx].specials[sym].name, (unsigned int)dst, *dst);
}

unsigned long get_var(unsigned int idx, int sym, long offset) {

	int *dst;
	unsigned long addr;

	if ((addr = vm_params[idx].specials[sym].addr) == -1) {
		BOOTER_PRINTF("\t%s not found.\n", vm_params[idx].specials[sym].name);
		return 0;
	}

	dst = (int *)(addr + offset);
	return *dst;
}

void set_net_phys_offset(unsigned int idx, long offset) {  // called with total_offset

	long *dst;
	unsigned long addr;

	if ((addr = vm_params[idx].specials[SPECIAL___boot_net_phys_offset__].addr) == -1) {
		BOOTER_PRINTF("\t__boot_net_phys_offset__ not found.\n");
		return;
	} else {
		BOOTER_PRINTF("\t__boot_net_phys_offset__ found @ 0x%08x\n", (unsigned int)addr);
	}
	dst = (long *)(addr + offset);

	/* FIXME: If __boot_net_phys_offset__ is used for anything except ANGEL_OFFSET_PTR() then va_angel needs to be handled differently */
	if (vm_params[idx].va_angel) {
		*dst = 0;
	} else {
		*dst = __boot_net_phys_offset__ + offset;
	}

	BOOTER_PRINTF("\tnet phys offset at 0x%08x set to <<0x%08x>>\n", (unsigned int)dst, (unsigned int) *dst);
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
		//		BOOTER_PRINTF("trans 0x%08lx -> 0x%09llx\n", va << H2K_KERNEL_ADDRBITS, pa << H2K_KERNEL_ADDRBITS);
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
		BOOTER_PRINTF("\t__clade_region_high_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_region_high_pd0_start__ found @ 0x%08x\n", (unsigned int)region_hi);
	}
	if (0 == region_hi) {  // unused weak symbol
		goto no_clade;
	}

	if ((comp = vm_params[idx].specials[SPECIAL___clade_comp_pd0_start__].addr) == -1) {
		BOOTER_PRINTF("\t__clade_comp_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_comp_pd0_start__ found @ 0x%08x\n", (unsigned int)comp);
	}

	if ((ex_lo_small = vm_params[idx].specials[SPECIAL___clade_exception_low_small_pd0_start__].addr) == -1) {
		BOOTER_PRINTF("\t__clade_exception_low_small_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_exception_low_small_pd0_start__ found @ 0x%08x\n", (unsigned int)ex_lo_small);
	}

	if ((ex_lo_large = vm_params[idx].specials[SPECIAL___clade_exception_low_large_pd0_start__].addr) == -1) {
		BOOTER_PRINTF("\t__clade_exception_low_large_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_exception_low_large_pd0_start__ found @ 0x%08x\n", (unsigned int)ex_lo_large);
	}

	if ((ex_hi_start = vm_params[idx].specials[SPECIAL___clade_exception_high_pd0_start__].addr) == -1) {
		BOOTER_PRINTF("\t__clade_exception_high_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_exception_high_pd0_start__ found @ 0x%08x\n", (unsigned int)ex_hi_start);
	}

	if ((ex_hi_end = vm_params[idx].specials[SPECIAL___clade_exception_high_pd0_end__].addr) == -1) {
		BOOTER_PRINTF("\t__clade_exception_high_pd0_end__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_exception_high_pd0_end__ found @ 0x%08x\n", (unsigned int)ex_hi_end);
	}

	if ((dict_start = vm_params[idx].specials[SPECIAL___clade_dict_pd0_start__].addr) == -1) {
		BOOTER_PRINTF("\t__clade_dict_pd0_start__ not found.\n");
		goto no_clade;
	} else {
		BOOTER_PRINTF("\t__clade_dict_pd0_start__ found @ 0x%08x\n", (unsigned int)dict_start);
	}

	/* Allocate a clade pd */
	vm_params[idx].clade_pd = pd_num++;
	if (pd_num > CLADE_NUM_PDS) {
		FAIL("\tOut of CLADE pds", "");
	}

	/* Copy high-prio exception data to TCM */
	ex_hi_size = ex_hi_end - ex_hi_start;
	if (ex_hi_size && tcm_size) {
		if (NULL == (vm_params[idx].clade_ex_hi = (unsigned int)h2_galloc(&tcm_alloc, ex_hi_size, 4096, 0))) {
			FAIL("\tgalloc ex_hi", "");
		}
		//		BOOTER_PRINTF("memcpy(0x%08x, 0x%08lx, 0x%08lx\n", vm_params[idx].clade_ex_hi, ex_hi_start + offset, ex_hi_size);
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
			FAIL("\tCLADE region address mismatch", "");
	}
	BOOTER_PRINTF("\tCLADE enabled\n");
	return;
 no_clade:
	BOOTER_PRINTF("\tCLADE not enabled\n");
}

void copy_vm(unsigned int idx, unsigned int offset) {

BOOTER_PRINTF("\tCopying from VM index %d:\n\t0x%09llx to 0x%09llx size 0x%09llx\n", idx, vm_params[idx].start_pa, vm_params[idx].start_pa + offset, vm_params[idx].end_pa - vm_params[idx].start_pa);
	memcpy((void *)(vm_params[idx].start_pa) + offset, (void *)(vm_params[idx].start_pa), vm_params[idx].end_pa - vm_params[idx].start_pa);
	dcclean_range((unsigned long)vm_params[idx].start_pa + offset, vm_params[idx].end_pa - vm_params[idx].start_pa);
}

void load_vm(unsigned int idx) {

	int fdesc, i, ret;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	int bytes_read;

	pcycles();

	int set_fence_lo = (~0L == vm_params[idx].fence_lo);
	int set_fence_hi = (0L == vm_params[idx].fence_hi);

	unsigned long heap_size, stack_size, total_size, prev_size, end, one_page, page_shift;
	unsigned long start = ~0L;
	int clone;
	unsigned long total_offset;

	char *elf = vm_params[idx].argv[0];

#ifdef BOOTVM_DEBUG
	int reads = 0;
#endif

	BOOTER_PRINTF("\n");  // FIXME: prepending \n to string results in an empty line in the output lately. Weird.
	BOOTER_PRINTF("Load VM index %d %s\n", idx, elf);

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

		vm_params[idx].offset_pages = (__boot_net_phys_offset__ + total_offset) >> PAGE_BITS;
		vm_params[idx].fence_lo = vm_params[clone].fence_lo + prev_size;
		vm_params[idx].fence_hi = vm_params[clone].fence_hi + prev_size;

		if (NULL != vm_params[clone].pmap && !vm_params[clone].pmap_added) { 
			vm_params[idx].pmap = vm_params[clone].pmap + prev_size;
		}

		copy_vm(clone, prev_size);
		set_net_phys_offset(idx, total_offset);

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

		if (0 > elf_get_specials(fdesc, vm_params[idx].specials, sizeof(vm_params[idx].specials)/sizeof(vm_params[idx].specials[0]), &ehdr)) {
			FAIL("\tCan't get special symbols", "");
		}

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
				vm_params[idx].offset_pages = (__boot_net_phys_offset__ + total_offset) >> PAGE_BITS;
			}
			phdr.p_paddr += vm_params[idx].load_offset;

			if (set_fence_lo && (phdr.p_paddr < vm_params[idx].fence_lo)) {
				vm_params[idx].fence_lo = phdr.p_paddr;
			}

			if (!vm_params[idx].skip_load) {
				BOOTER_PRINTF("\tload VA %08lx at %08lx\n", (unsigned long)phdr.p_vaddr, (unsigned long)phdr.p_paddr + __boot_net_phys_offset__);
				bytes_read = 0;
				do {
					bytes_read += ret = read(fdesc,(char *)phdr.p_paddr + bytes_read, phdr.p_filesz - bytes_read);
#ifdef BOOTVM_DEBUG
					reads++;
#endif					
				} while (ret > 0);
				if (ret == -1) {
					error("\tCan't read() in ", elf);
				}

				memset((char *)phdr.p_paddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
				/* Really, only need to clean out text sections */
				dcclean_range(phdr.p_paddr, phdr.p_memsz);
			}
		}
#ifdef BOOTVM_DEBUG
		BOOTER_PRINTF("\t\tTotal read operations: %d\n", reads);
#endif		
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
		set_var(idx, SPECIAL___sys_write_mode__, vm_params[idx].sys_write_mode, total_offset);

		get_pmap(idx, total_offset);
		// if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;

		/* Adjust guest_base and fences */
		if (-1 == (end = vm_params[idx].specials[SPECIAL_end].addr)) {
			FAIL("\tCan't find end symbol", "");
		}
		BOOTER_PRINTF("\tend 0x%08lx\n", end);

		vm_params[idx].start_pa = start + total_offset;
		vm_params[idx].end_pa = end + total_offset;
		dcclean_range((unsigned long)vm_params[idx].start_pa, vm_params[idx].end_pa - vm_params[idx].start_pa);

		heap_size = vm_params[idx].specials[SPECIAL_HEAP_SIZE].addr;
		if (0 == heap_size || -1 == heap_size) {
			if (0 == vm_params[idx].specials[SPECIAL_DEFAULT_HEAP_SIZE].addr
					|| -1 == vm_params[idx].specials[SPECIAL_DEFAULT_HEAP_SIZE].addr) {
				heap_size = GUESS_HEAP_SIZE;
				BOOTER_PRINTF("\t** warning: heap size unknown, guessing 0x%08lx\n", heap_size);
			} else {
				heap_size = vm_params[idx].specials[SPECIAL_DEFAULT_HEAP_SIZE].addr;
				BOOTER_PRINTF("\theap_size 0x%08lx (DEFAULT_HEAP_SIZE)\n", heap_size);
			}
		} else {
			BOOTER_PRINTF("\theap_size 0x%08lx\n", heap_size);
		}

		stack_size = vm_params[idx].specials[SPECIAL_STACK_SIZE].addr;
		if (0 == stack_size || -1 == stack_size) {
			if (0 == vm_params[idx].specials[SPECIAL_DEFAULT_STACK_SIZE].addr
					|| -1 == vm_params[idx].specials[SPECIAL_DEFAULT_STACK_SIZE].addr) {
				stack_size = GUESS_STACK_SIZE;
				BOOTER_PRINTF("\t** warning: stack size unknown, guessing 0x%08lx\n", stack_size);
			} else {
				stack_size = vm_params[idx].specials[SPECIAL_DEFAULT_STACK_SIZE].addr;
				BOOTER_PRINTF("\tstack_size 0x%08lx (DEFAULT_STACK_SIZE)\n", stack_size);
			}
		} else {
			BOOTER_PRINTF("\tstack_size 0x%08lx\n", stack_size);
		}

		end += heap_size + stack_size;
		vm_params[idx].stack = (void *)(end & -32);  // should be close to where crt0 puts the stack

		end = H2_ALIGN_UP(end, one_page);
		vm_params[idx].start_va = start;
		vm_params[idx].end_va = end;

		total_size = H2_ALIGN_UP((end - start), one_page);
		vm_params[idx].pages = total_size >> page_shift;

		BOOTER_PRINTF("\ttotal_size 0x%08lx\n", total_size);
		guest_base += total_size;

		if (set_fence_lo) {
			vm_params[idx].fence_lo += __boot_net_phys_offset__;
			vm_params[idx].fence_lo &= HI_MASK(one_page);
		}
		if (set_fence_hi) {
			if (tight_fence_hi) {
				vm_params[idx].fence_hi = vm_params[idx].fence_lo + total_size - one_page;
			} else {
				vm_params[idx].fence_hi = FENCE_HI_MAX & HI_MASK(one_page);
			}
		}
		if (vm_params[idx].stash) {
			BOOTER_PRINTF("\tStashing:\n\t");
			vm_params[idx].stash_addr = __boot_net_phys_offset__ + guest_base;
			guest_base += vm_params[idx].end_pa - vm_params[idx].start_pa;
			guest_base = H2_ALIGN_UP(guest_base, one_page);
			copy_vm(idx, vm_params[idx].stash_addr - vm_params[idx].start_pa);
		}
	}  // else not a clone

	clade_setup(idx, total_offset);

	BOOTER_PRINTF("\tentry 0x%08lx\n", (unsigned long)vm_params[idx].entry);
	BOOTER_PRINTF("\tphys_offset 0x%08lx\n", vm_params[idx].phys_offset);
	BOOTER_PRINTF("\tload_offset 0x%08lx\n", vm_params[idx].load_offset);
	BOOTER_PRINTF("\toffset_pages 0x%08lx\n", vm_params[idx].offset_pages);

	if (vm_params[idx].pmap_added) {
		BOOTER_PRINTF("\tguest translations added\n");
	} else {
		BOOTER_PRINTF("\tfence_lo 0x%08x\n", vm_params[idx].fence_lo);
		BOOTER_PRINTF("\tfence_hi 0x%08x\n", vm_params[idx].fence_hi);
	}
}

void config_vm(unsigned int idx) {

	unsigned long vm;

	H2K_offset_t base;
	int trans;
	

	BOOTER_PRINTF("\n");
	BOOTER_PRINTF("Config VM index %d\n", idx);
	BOOTER_PRINTF("\tVirtual CPUs %d\n", vm_params[idx].num_vcpus);
	BOOTER_PRINTF("\tShared interrupts  %d\n", vm_params[idx].num_shared_ints);
	BOOTER_PRINTF("\tPriority  %d\n", vm_params[idx].startprio);

	vm = h2_config_vmblock_init(0, SET_CPUS_INTS, CONFIG_CPUS(vm_params[idx].use_ext, vm_params[idx].num_vcpus), vm_params[idx].num_shared_ints);

	base.size = vm_params[idx].page_size;
	base.cccc = vm_params[idx].cccc;
	base.xwru = vm_params[idx].xwru;
	base.pages = vm_params[idx].offset_pages;

	if (-1 == vm_params[idx].trans_type) {  // not set on cmdline
		if (NULL != vm_params[idx].pmap) {  // has __guest_pmap__
			trans = vm_params[idx].pmap->type;
		  base.raw = vm_params[idx].pmap->base.raw;
			BOOTER_PRINTF("\tGuest pmap type %s\n", trans_name[trans]);
			BOOTER_PRINTF("\tGuest pmap base 0x%08x\n", base.raw);
		} else {  // default
			trans = H2K_ASID_TRANS_TYPE_OFFSET;
			BOOTER_PRINTF("\tTranslation type offset\n");
			BOOTER_PRINTF("\t\tPage size %d (%s)\n", base.size, pagesize_name[base.size]);
			BOOTER_PRINTF("\t\tCCCC 0x%1x\n", base.cccc);
			BOOTER_PRINTF("\t\tXWRU 0x%1x\n", base.xwru);
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

	BOOTER_PRINTF("\tPriority %d\n", vm_params[idx].bestprio);
	BOOTER_PRINTF("\tTrapmask 0x%08x\n", vm_params[idx].trapmask);
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
	BOOTER_PRINTF("\tVM ID %lu\n", vm);
}

void boot_vm(unsigned int idx) {

	unsigned int regval;
	int bit = 0;
	int shift;
	unsigned long total_offset = vm_params[idx].phys_offset + vm_params[idx].load_offset;

	BOOTER_PRINTF("\n");
	BOOTER_PRINTF("Boot VM index %d, ID %d\n", idx, vm_params[idx].id);

	if (~0L != vm_params[idx].ccr) {
		regval = H2K_get_ccr();
		BOOTER_PRINTF("\told value for ccr: 0x%08x\n",regval);
		H2K_set_ccr(vm_params[idx].ccr);
		regval = H2K_get_ccr();
		BOOTER_PRINTF("\tnew value for ccr: 0x%08x\n",regval);
	}
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	int err;
	if (~0L != vm_params[idx].vwctrl) {
		regval = h2_hwconfig_vwctrl_get();
		BOOTER_PRINTF("\told value for vwctrl: 0x%08x\n",regval);
		if ((err = h2_hwconfig_vwctrl_set(vm_params[idx].vwctrl))) {
			FAIL("\tCan't set vwctrl\n", "");
		}
		regval = h2_hwconfig_vwctrl_get();
		BOOTER_PRINTF("\tnew value for vwctrl: 0x%08x\n",regval);
	}
#endif
	if (vm_params[idx].pmu_sweep) {
		if (vm_params[idx].pmu_overflow) {  // 64-bit counters
			pmu_evtcfg =  0x02000100;  // COUNTER2_OVERFLOW ... COUNTER0_OVERFLOW
			pmu_evtcfg1 = 0x06000500;  // COUNTER6_OVERFLOW ... COUNTER4_OVERFLOW
			shift = 16;
		} else {
			pmu_evtcfg = pmu_evtcfg1 = 0;  // off
			shift = 8;
		}
		pmu_cfg = 0;
		while ((vm_params[idx].pmu_first_event <= vm_params[idx].pmu_last_event) && bit < 64) {
			switch (vm_params[idx].pmu_first_event ) {
			case PMU_COUNTER_NONE:
			case PMU_COUNTER0_OVERFLOW:
			case PMU_COUNTER2_OVERFLOW:
			case PMU_COUNTER4_OVERFLOW:
			case PMU_COUNTER6_OVERFLOW:
				vm_params[idx].pmu_first_event++;  // skip
				break;
			default:
				if (bit < 32) {
					pmu_cfg |= (vm_params[idx].pmu_first_event >> 8) << (bit / 4);
					pmu_evtcfg |= (vm_params[idx].pmu_first_event++ & 0xff) << bit;
				} else {
					pmu_cfg |= (vm_params[idx].pmu_first_event >> 8) << (bit / 4);
					pmu_evtcfg1 |= (vm_params[idx].pmu_first_event++ & 0xff) << (bit - 32);
				}
				bit += shift;
			}
		}
	}
	
	/* FIXME: For now set only in first vm */
	if (0 == idx) {
		if (set_pmu_evtcfg) {
			set_var(idx, SPECIAL___h2_pmu_evtcfg__, pmu_evtcfg, total_offset);
		}
		if (set_pmu_evtcfg1) {
			set_var(idx, SPECIAL___h2_pmu_evtcfg1__, pmu_evtcfg1, total_offset);
		}
		if (set_pmu_cfg) {
			set_var(idx, SPECIAL___h2_pmu_cfg__, pmu_cfg, total_offset);
		}
		if (gpio_toggle) {
			set_var(idx, SPECIAL___h2_gpio_toggle__, gpio_toggle, total_offset);
		}
	}

	pcycles();

	if (-1 == h2_vmboot(vm_params[idx].entry, vm_params[idx].stack, vm_params[idx].arg, vm_params[idx].startprio, vm_params[idx].id) ) {
		FAIL("\tfailed to boot vm\n", "");
	}
}

void dump_pmu(int idx) {
	int i;
	unsigned long long int val;
	int event;
	unsigned long total_offset = vm_params[idx].phys_offset + vm_params[idx].load_offset;

	if (!pmu_dump) return;

	/* FIXME: For now, dump only first vm */
	if (0 != idx) return;

	/* In case someone has messed with __h2_pmu_* while we were napping */
	pmu_evtcfg = get_var(idx, SPECIAL___h2_pmu_evtcfg__, total_offset);
	pmu_evtcfg1 = get_var(idx, SPECIAL___h2_pmu_evtcfg1__, total_offset);
	pmu_cfg = get_var(idx, SPECIAL___h2_pmu_cfg__, total_offset);

	if (vm_params[idx].pmu_overflow) {
		for (i = 0; i < 8; i += 2) {
			if (i < 4) {
				event = (pmu_evtcfg >> (i * 8)) & 0xff;
			} else {
				event = (pmu_evtcfg1 >> ((i - 4) * 8)) & 0xff;
			}
			event |= ((pmu_cfg >> (i * 2)) & 0x3) << 8;

			val = h2_pmu_getreg(H2_PMUCNT0 + i);
			val |= ((unsigned long long int)h2_pmu_getreg(H2_PMUCNT0 + i + 1)) << 32;
			if (event != PMU_COUNTER_NONE) {
				pmu_idx += snprintf(pmu_buf + pmu_idx, PMU_BUFSIZE - idx, "0x%x\t: %lld\n", event, val);
			}
		}
	} else {
		for (i = 0; i < 8; i++) {
			if (i < 4) {
				event = (pmu_evtcfg >> (i * 8)) & 0xff;
			} else {
				event = (pmu_evtcfg1 >> ((i - 4) * 8)) & 0xff;
			}
			event |= ((pmu_cfg >> (i * 2)) & 0x3) << 8;

			val = h2_pmu_getreg(H2_PMUCNT0 + i);
			if (event != PMU_COUNTER_NONE) {
				pmu_idx += snprintf(pmu_buf + pmu_idx, PMU_BUFSIZE - idx, "0x%x\t: %lld\n", event, val);
			}
		}
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
	if (pmu_dump) {
		if (NULL == (pmu_fp = fopen(pmu_file, "w"))) {
			error("Can't open ", pmu_file);
		}
	}

	/* Reset PMU */
	if (0 != h2_pmu_setreg(H2_PMUEVTCFG, 0)) error("h2_pmu_setreg(H2_PMUEVTCFG, 0)", NULL);
	if (0 != h2_pmu_setreg(H2_PMUEVTCFG1, 0)) error("h2_pmu_setreg(H2_PMUEVTCFG1, 0)", NULL);
	if (0 != h2_pmu_setreg(H2_PMUCFG, 0)) error("h2_pmu_setreg(H2_PMUCFG, 0)", NULL);
	for (i = 0; i < 8; i++) {
		if (0 != h2_pmu_setreg(H2_PMUCNT0 + i, 0)) error("h2_pmu_setreg(H2_PMUCNTx, 0)", NULL);
	}
	/* sim/emu PMU stats reset */
	asm volatile (" r0 = #0x48 ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory");

	for (i = 0 ; i <= idx; i++) {
		boot_vm(i);
	}

	/* Wait for all VMs to stop or error */
	BOOTER_PRINTF("\n");
	BOOTER_PRINTF("booter: Waiting for interrupts\n");

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
					BOOTER_PRINTF("Sample -- hthread: %d  ID: 0x%08x  ELR: 0x%08x\n", i, (unsigned int)(res[i] >> 32), (unsigned int)(res[i] & 0xffffffff));
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
				BOOTER_PRINTF("VM %d status 0x%x\n", vm, status);
				cpus = h2_vmstatus(VMOP_STATUS_CPUS, vm);
				BOOTER_PRINTF("VM %d Live CPUs: %d\n", vm, cpus);

				if (0 == cpus) {  // no more cpus running
					if (status != vm_params[i].expect_status && vm_params[i].error_exit) {
						FAIL("\tUnexpected exit status.", "");
					}
					vm_params[i].boots = (vm_params[i].boots > 0 ? vm_params[i].boots - 1 : 0);
					if (vm_params[i].boots
							|| (vm_params[i].pmu_sweep
									&& (vm_params[i].pmu_first_event <= vm_params[i].pmu_last_event))) {  // reboot
						done = 0;
						dump_pmu(i);
						if (vm_params[i].reboot_reload) {
							load_vm(i);
						}
						if (vm_params[i].stash) {
							BOOTER_PRINTF("Restoring VM index %d: 0x%09llx to 0x%09llx size 0x%09llx\n", idx, vm_params[idx].stash_addr, vm_params[idx].start_pa, vm_params[idx].end_pa - vm_params[idx].start_pa);
							memcpy((void *)(vm_params[idx].start_pa), (void *)(vm_params[idx].stash_addr), vm_params[idx].end_pa - vm_params[idx].start_pa);
							dcclean_range((unsigned long)vm_params[idx].start_pa, vm_params[idx].end_pa - vm_params[idx].start_pa);
							/* if this vm was cloned from another, need to set its net phys offset again (overwritten by restore from stash) */
							set_net_phys_offset(idx, vm_params[idx].phys_offset + vm_params[idx].load_offset);  // i.e. set_net_phys_offset(idx, total_offset)
						}
						boot_vm(i);
					} else {  // all done with this VM
						dump_pmu(i);
						vm_params[i].id = ~0;  // mark non-existent
						h2_vmfree(vm);
						pcycles();
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

	if (pmu_dump) {
		fprintf(pmu_fp, "%s", pmu_buf);
		fclose(pmu_fp);
	}
}

void die_usage()
{
	usage();
	exit(1);
}

void print_infos() {

	unsigned int unit;
	pcycles();

	BOOTER_PRINTF("\n");
	BOOTER_PRINTF("H2/core info:\n");
	BOOTER_PRINTF("\tBuild ID: 0x%08x\n", h2_info(INFO_BUILD_ID));
	BOOTER_PRINTF("\tGuest PC sampling available: %s\n", (boot_flags.boot_have_sample ? "true" : "false"));
#ifdef MULTICORE
	BOOTER_PRINTF("\tMulti-core:\n");
	BOOTER_PRINTF("\t\tCount ( 0 == single core) %d\n", core_count);
	BOOTER_PRINTF("\t\tID %d\n", core_id);
	BOOTER_PRINTF("\t\tShift 0x%08x\n", h2_info(INFO_SHIFT));
	BOOTER_PRINTF("\t\tTCM base offset 0x%08x\n", h2_info(INFO_TCM_OFFSET));
#endif

	BOOTER_PRINTF("\tCoprocessors:\n");
	unit = h2_info(INFO_UNIT_START);
	if (0 != unit) {  // have new unit cfg blocks
		while (unit) {
			if (h2_info_unit(unit, CFG_UNIT_ID) == CFG_TYPE_VXU0) {
				BOOTER_PRINTF("\t  VXU0 subtype %x:\n", h2_info_unit(unit, CFG_UNIT_SUBID));
				BOOTER_PRINTF("\t    Unit ID %x:\n", h2_info_unit(unit, CFG_VXU_UNIT_ID));
				BOOTER_PRINTF("\t    HVX contexts: 0x%08x\n", h2_info_unit(unit, CFG_HVX_CONTEXTS));
				BOOTER_PRINTF("\t    HLX contexts: 0x%08x\n", h2_info_unit(unit, CFG_HLX_CONTEXTS));
				BOOTER_PRINTF("\t    HMX contexts: 0x%08x\n", h2_info_unit(unit, CFG_HMX_CONTEXTS));
				BOOTER_PRINTF("\t    HVX vector length: %d bytes\n", h2_info_unit(unit, CFG_HVX_VEC_LENGTH));
				BOOTER_PRINTF("\t    HLX register length: %d bytes\n", h2_info_unit(unit, CFG_HLX_REG_LENGTH));
				BOOTER_PRINTF("\t    VTCM base: 0x%08x\n", h2_info_unit(unit, CFG_VTCM_BASE) << 16);
				BOOTER_PRINTF("\t    VTCM size: %dK\n", h2_info_unit(unit, CFG_VTCM_SIZE));
			}
			unit = h2_info_unit(unit, CFG_UNIT_NEXT);
		}
	} else { // old style
		BOOTER_PRINTF("\t  HLX:\n");
		BOOTER_PRINTF("\t    Present: %s\n", (boot_flags.boot_have_hlx ? "true" : "false"));
		if (boot_flags.boot_have_hlx) {
			BOOTER_PRINTF("\t    Contexts : %d\n", h2_info(INFO_HLX_CONTEXTS));
		}
		BOOTER_PRINTF("\t  HVX:\n");
		BOOTER_PRINTF("\t    Present: %s\n", (boot_flags.boot_have_hvx ? "true" : "false"));
		if (boot_flags.boot_have_hvx) {
			BOOTER_PRINTF("\t    Native vector length: %d\n", h2_info(INFO_HVX_VLENGTH));
			BOOTER_PRINTF("\t    Contexts (when v2x == 0): %d\n", h2_info(INFO_COPROC_CONTEXTS));
			BOOTER_PRINTF("\t    Can context-switch in kernel: %s\n", (boot_flags.boot_ext_ok ? "true" : "false"));
#if ARCHV >= 65
			BOOTER_PRINTF("\t    VTCM base: 0x%08x\n", h2_info(INFO_VTCM_BASE));
			BOOTER_PRINTF("\t    VTCM size: %dK\n", h2_info(INFO_VTCM_SIZE));
#endif
		}
#if ARCHV >= 68
		BOOTER_PRINTF("\t  HMX:\n");
		BOOTER_PRINTF("\t    Present: %s\n", (boot_flags.boot_have_hmx ? "true" : "false"));
		if (boot_flags.boot_have_hmx) {
			BOOTER_PRINTF("\t    Units: %d\n", h2_info(INFO_HMX_INSTANCES));
		}
		BOOTER_PRINTF("\tUser-mode DMA present: ");
		BOOTER_PRINTF((boot_flags.boot_have_dma ? "true\n" : "false\n"));
#endif
	}
#ifdef CLUSTER_SCHED
	unsigned int max_coprocs = h2_info(INFO_MAX_CLUSTER_COPROC);
	if (max_coprocs == -1) {
			BOOTER_PRINTF("\tCluster scheduling disabled\n");
		} else {
			BOOTER_PRINTF("\tMax coprocessor threads per cluster: %d\n", h2_info(INFO_MAX_CLUSTER_COPROC));
		}
#endif
	BOOTER_PRINTF("\tSILVER present: ");
	BOOTER_PRINTF((boot_flags.boot_have_silver ? "true\n" : "false\n"));
	BOOTER_PRINTF("\tKernel physical address: 0x%08x\n", h2_info(INFO_PHYSADDR));
	BOOTER_PRINTF("\tKernel page size: %dK\n", h2_info(INFO_H2K_PGSIZE) / 1024);
	BOOTER_PRINTF("\tNumber of kernel pages: %d\n", h2_info(INFO_H2K_NPAGES));
	BOOTER_PRINTF("\tH2 kernel in TCM: %s\n", (boot_flags.boot_use_tcm ? "true" : "false"));
	BOOTER_PRINTF("\tcfgbase: 0x%08x\n", h2_info(INFO_CFGBASE));
	BOOTER_PRINTF("\tTCM (adjusted) base: 0x%08x\n", tcm_base);
	BOOTER_PRINTF("\tTCM (remaining) size: %dK\n", tcm_size / 1024);
	BOOTER_PRINTF("\tL2 array size: %dK\n", h2_info(INFO_L2MEM_SIZE) / 1024);
	BOOTER_PRINTF("\tL2 cache size: %dK\n", h2_info(INFO_L2TAG_SIZE) / 1024);
	BOOTER_PRINTF("\tL2 line size: %d\n", h2_info(INFO_L2_LINE_SZ));
	BOOTER_PRINTF("\tL2 register base: 0x%08x\n", h2_info(INFO_L2CFG_BASE));
	BOOTER_PRINTF("\tCLADE register base: 0x%08x\n", clade_base);
#if ARCHV >= 65
	BOOTER_PRINTF("\tECC register base: 0x%08x\n", h2_info(INFO_ECC_BASE));
	BOOTER_PRINTF("\tAudio extension: %d\n", h2_info(INFO_AUDIO_EXT));
#endif
	BOOTER_PRINTF("\tTLB entries: %d\n", h2_info(INFO_TLB_SIZE));
	BOOTER_PRINTF("\tReplaceable TLB entries: %d\n", h2_info(INFO_TLB_FREE));
	BOOTER_PRINTF("\tSTLB: %s\n", (stlb_info.stlb_enabled ? "true" : "false"));
	BOOTER_PRINTF("\t\tEnabled: %s\n", (stlb_info.stlb_enabled ? "true" : "false"));
	if (stlb_info.stlb_enabled) {
		BOOTER_PRINTF("\t\tSets per ASID: %d\n", 1 << stlb_info.stlb_max_sets_log2);
		BOOTER_PRINTF("\t\tWays: %d\n", stlb_info.stlb_max_ways);
		BOOTER_PRINTF("\t\tSize: %d\n", stlb_info.stlb_size);
		BOOTER_PRINTF("\t\tEntries: %dK\n", ((1 << stlb_info.stlb_max_sets_log2) * stlb_info.stlb_max_ways * stlb_info.stlb_size) / 1024);
	}
	BOOTER_PRINTF("\tsyscfg: 0x%08x\n", h2_info(INFO_SYSCFG));
	BOOTER_PRINTF("\trev: 0x%08x\n", h2_info(INFO_REV));
	BOOTER_PRINTF("\tSubsystem base: 0x%08x\n", h2_info(INFO_SSBASE));
	BOOTER_PRINTF("\tL2VIC physical base: 0x%08x\n", h2_info(INFO_L2VIC_BASE));
	BOOTER_PRINTF("\tTimer physical base: 0x%08x\n", h2_info(INFO_TIMER_BASE));
	BOOTER_PRINTF("\tTimer interrupt: %d\n", h2_info(INFO_TIMER_INT));
	BOOTER_PRINTF("\tRunning HW threads mask: 0x%08x\n", h2_info(INFO_HTHREADS));
}

void kernel_setup() {

	int val, offset;

	if (ecc_enable != -1) {
		BOOTER_PRINTF("ECC regs old values:\n");
		for (offset = 0; offset <= ECCREGS_MAX; offset += ECCREGS_STRIDE) {
			BOOTER_PRINTF("\toffset 0x%03x: 0x%08x\n", offset, h2_hwconfig_ecc_get_reg(offset));
		}
		if (h2_hwconfig_ecc(ecc_enable) < 0) {
			FAIL("ECC enable", "");
		}
		BOOTER_PRINTF("ECC regs new values:\n");
		for (offset = 0; offset <= ECCREGS_MAX; offset += ECCREGS_STRIDE) {
			BOOTER_PRINTF("\toffset 0x%03x: 0x%08x\n", offset, h2_hwconfig_ecc_get_reg(offset));
		}
	}

	if (hwt_num != -1) {
		if (hwt_mask != -1) {
			FAIL("Can't set both hwt_mask and hwt_num", "");
		}
		if (h2_hwconfig_hwthreads_num(hwt_num) < 0) {
			FAIL("hwthreads_num", "");
		}
	} else {
		if (h2_hwconfig_hwthreads_mask(hwt_mask) < 0) {
			FAIL("hwthreads_mask", "");
		}
	}

	if (use_stlb) {
		if (h2_config_stlb_alloc() < 0) {
			FAIL("STLB alloc", "");
		}
	}

	if (boot_flags.boot_have_dma) {
		if (set_dmactrl) {
			val = dmactrl;
		} else {
			if ((val = h2_hwconfig_dma_getcfg(DMA_CTRL_SET_INDEX)) == -1) {
				FAIL("dma_getcfg", "");
			}
			/* Disable DMA stall in guest mode */
			val = DMA_CTRL_SET_VAL;
		}
		if (h2_hwconfig_dma_setcfg(DMA_CTRL_SET_INDEX, val) == -1) {
			FAIL("dma_setcfg", "");
		}
	} else {
		if (set_dmactrl) {
			FAIL("Can't set DMA control reg -- DMA not present", "");
		}
	}

#if HAVE_EXTENSIONS
	if (ext_power) {
		if (h2_hwconfig_extpower(1) < 0) {
			FAIL("extpower", "");
		}
	}
#endif

#ifdef CLUSTER_SCHED
	if (cluster_sched) {
		if (h2_config_cluster_sched(1) < 0) {
			FAIL("Cluster sched", "");
		}
	}
#endif	
}

void get_l2_reg(unsigned int idx) {
	unsigned int val, kerror;
	
	val = h2_hwconfig_l2_get_reg(getl2reg_offsets[idx]);
	kerror = h2_info(INFO_ERROR);
	if (kerror != KERROR_NONE) {
		BOOTER_PRINTF("\n");
		BOOTER_PRINTF("Kernel error: %s\n\n", kerror_msg[kerror]);
		FAIL("Can't get L2 reg.", "");
	}
	BOOTER_PRINTF("L2 reg at offset 0x%08x: 0x%08x\n", getl2reg_offsets[idx], val);
}

void set_l2_reg(unsigned int offset, unsigned int val) {

  unsigned int old, ret, kerror;

	BOOTER_PRINTF("Set L2 reg at offset 0x%08x:\n", offset);

	old = h2_hwconfig_l2_get_reg(offset);

	kerror = h2_info(INFO_ERROR);
	if (kerror != KERROR_NONE) {
		BOOTER_PRINTF("\n");
		BOOTER_PRINTF("Kernel error: %s\n\n", kerror_msg[kerror]);
		FAIL("Can't get L2 reg.", "");
	}

	BOOTER_PRINTF("\tOld value:  0x%08x\n", old);
	ret = h2_hwconfig_l2_set_reg(offset, val);

	if (ret != old) {
		FAIL("set_l2_reg mismatch.", "");
	}

	BOOTER_PRINTF("\tNew value:  0x%08x\n", val);
}

void set_stride_prefetcher_reg(unsigned int offset, unsigned int val) {

  unsigned int old, ret, kerror;

	BOOTER_PRINTF("Set stride prefetcher reg at offset 0x%08x:\n", offset);

	old = h2_hwconfig_strideprefetcher_get_reg(offset);

	kerror = h2_info(INFO_ERROR);
	if (kerror != KERROR_NONE) {
		BOOTER_PRINTF("\n");
		BOOTER_PRINTF("Kernel error: %s\n\n", kerror_msg[kerror]);
		FAIL("Can't get stride prefetcher reg.", "");
	}

	BOOTER_PRINTF("\tOld value:  0x%08x\n", old);
	ret = h2_hwconfig_strideprefetcher_set_reg(offset, val);

	if (ret != old) {
		FAIL("set_stride_prefetcher_reg mismatch.", "");
	}

	BOOTER_PRINTF("\tNew value:  0x%08x\n", val);
}

void set_dvlm_reg(unsigned int offset, unsigned int val) {

  unsigned int old, ret, kerror;

	BOOTER_PRINTF("Set DPM volt reg at offset 0x%08x:\n", offset);

	old = h2_hwconfig_dpm_voltlmtmgmt_get_reg(offset);

	kerror = h2_info(INFO_ERROR);
	if (kerror != KERROR_NONE) {
		BOOTER_PRINTF("\n");
		BOOTER_PRINTF("Kernel error: %s\n\n", kerror_msg[kerror]);
		FAIL("Can't get DPM volt reg.", "");
	}

	BOOTER_PRINTF("\tOld value:  0x%08x\n", old);
	ret = h2_hwconfig_dpm_voltlmtmgmt_set_reg(offset, val);

	if (ret != old) {
		FAIL("set_dvlm_reg mismatch.", "");
	}

	BOOTER_PRINTF("\tNew value:  0x%08x\n", val);
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
	BOOTER_PRINTF("Unknown SYSCFG bit %s\n", name);
	FAIL("set_syscfg_field", "");
}

extern void bootvm_vectors();

void booter_isr(unsigned int gssr) {

	if ((gssr & 0xff) == H2K_VM_CHILDINT) {
		//		BOOTER_PRINTF("Got child interrupt\n");
		h2_anysignal_set(&wake_sig, WAKE_CHILD);
		h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_VM_CHILDINT, 0);
	}
	if ((gssr & 0xff) == H2K_TIME_GUESTINT) {
		//		BOOTER_PRINTF("Got timer interrupt\n");
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
#ifdef MULTICORE
		int count = 0;
		char **save;
		if (0 == strcmp(argv[0], "--core")) {
			if (strtoul(argv[1],NULL,0) != core_id) {  // not this core
				argc -= 2; argv += 2;
				while (argc > 0 && 0 != strcmp(argv[0], "--core")) {  // skip to next --core
					argc -= 1; argv += 1;
				}
			} else {  // is this core, adjust argc
				argc -= 2; argv += 2; save = argv;
				while (argc > 0 && 0 != strcmp(argv[0], "--core")) {  // count to next --core
					count += 1; argc -= 1; argv += 1;
				}
				argc = count;
				argv = save;
			}
			continue;
		} else
#endif
		if (0 == strcmp(argv[0],"--quiet")) {
#ifdef MULTICORE
			silent |= strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
#else
			silent = 1;
			argc -= 1; argv += 1;
#endif
			continue;
		} else if (0 == strcmp(argv[0],"--cycles")) {
			cycles = 1;
			argc -= 1; argv += 1;
			continue;
		} else if (0 == strcmp(argv[0],"--syscfg")) {
			if (argc < 2) die_usage();
			regval = h2_info(INFO_SYSCFG);
			BOOTER_PRINTF("Old value for syscfg: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_syscfg(regval);
			regval = h2_info(INFO_SYSCFG);
			BOOTER_PRINTF("New value for syscfg: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--livelock")) {
			if (argc < 2) die_usage();
			regval = h2_info(INFO_LIVELOCK);
			BOOTER_PRINTF("Old value for livelock: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_livelock(regval);
			regval = h2_info(INFO_LIVELOCK);
			BOOTER_PRINTF("New value for livelock: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--turkey")) {
			if (argc < 2) die_usage();
			regval = H2K_get_turkey();
			BOOTER_PRINTF("Old value for turkey: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_turkey(regval);
			regval = H2K_get_turkey();
			BOOTER_PRINTF("New value for turkey: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--turkey")) {
			if (argc < 2) die_usage();
			regval = H2K_get_turkey();
			printf("Old value for turkey: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_turkey(regval);
			regval = H2K_get_turkey();
			printf("New value for turkey: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--duck")) {
			if (argc < 2) die_usage();
			regval = H2K_get_duck();
			BOOTER_PRINTF("Old value for duck: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_duck(regval);
			regval = H2K_get_duck();
			BOOTER_PRINTF("New value for duck: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--chicken")) {
			if (argc < 2) die_usage();
			regval = H2K_get_chicken();
			BOOTER_PRINTF("Old value for chicken: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_chicken(regval);
			regval = H2K_get_chicken();
			BOOTER_PRINTF("New value for chicken: 0x%08x\n",regval);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0],"--rgdr")) {
			if (argc < 2) die_usage();
			regval = H2K_get_rgdr();
			BOOTER_PRINTF("Old value for rgdr: 0x%08x\n",regval);
			regval = strtoul(argv[1],NULL,0);
			H2K_set_rgdr(regval);
			regval = H2K_get_rgdr();
			BOOTER_PRINTF("New value for rgdr: 0x%08x\n",regval);
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

		} else if (0 == strcmp(argv[0], "--dma_ctrl")) {
			if (argc < 2) die_usage();
			dmactrl = strtoul(argv[1], NULL, 0);
			set_dmactrl = 1;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--l2_reg")) {
			if (argc < 3) die_usage();
			set_l2_reg(strtoul(argv[1], NULL, 0), strtoul(argv[2], NULL, 0));
			argc -= 3; argv += 3;
			continue;

		} else if (0 == strcmp(argv[0], "--get_l2_reg")) {
			if (argc < 3) die_usage();
			if (NULL == (getl2reg_offsets = realloc(getl2reg_offsets, sizeof(unsigned int) * (getl2reg + 1)))) {
				error("realloc getl2reg_offsets", NULL);
			}
			getl2reg_offsets[getl2reg] = strtoul(argv[1], NULL, 0);
			get_l2_reg(getl2reg);
			getl2reg++;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--stride_prefetch_reg")) {
			if (argc < 3) die_usage();
			set_stride_prefetcher_reg(strtoul(argv[1], NULL, 0), strtoul(argv[2], NULL, 0));
			argc -= 3; argv += 3;
			continue;

		} else if (0 == strcmp(argv[0], "--dvlm_reg")) {
			if (argc < 3) die_usage();
			set_dvlm_reg(strtoul(argv[1], NULL, 0), strtoul(argv[2], NULL, 0));
			argc -= 3; argv += 3;
			continue;

#ifdef HAVE_EXTENSIONS
		} else if (0 == strcmp(argv[0], "--ext_power")) {
			if (argc < 2) die_usage();
			ext_power = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--hmx_poweron_addr")) {
			if (argc < 2) die_usage();
			if (h2_hwconfig_hmxpower_on_set_addr(strtoul(argv[1],NULL,0)) == -1) {
				FAIL("HWCONFIG_HMX_POWERON_ADDR", "");
			}
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--hmx_poweroff_addr")) {
			if (argc < 2) die_usage();
			if (h2_hwconfig_hmxpower_off_set_addr(strtoul(argv[1],NULL,0)) == -1) {
				FAIL("HWCONFIG_HMX_POWEROFF_ADDR", "");
			}
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

		} else if (0 == strcmp(argv[0], "--hwt_mask")) {
			if (argc < 2) die_usage();
			hwt_mask = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--hwt_num")) {
			if (argc < 2) die_usage();
			hwt_num = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--ecc_enable")) {
			if (argc < 2) die_usage();
			ecc_enable = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_evtcfg")) {
			if (argc < 2) die_usage();
			pmu_evtcfg = strtoul(argv[1],NULL,0);
			set_pmu_evtcfg = 1;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_evtcfg1")) {
			if (argc < 2) die_usage();
			pmu_evtcfg1 = strtoul(argv[1],NULL,0);
			set_pmu_evtcfg1 = 1;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_cfg")) {
			if (argc < 2) die_usage();
			pmu_cfg = strtoul(argv[1],NULL,0);
			set_pmu_cfg = 1;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_dump")) {
			if (argc < 2) die_usage();
			pmu_dump = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_file")) {
			if (argc < 2) die_usage();
			pmu_file = argv[1];
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--gpio_toggle")) {
			if (argc < 2) die_usage();
			gpio_toggle = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--gpio_addr")) {
			if (argc < 2) die_usage();
			if (h2_hwconfig_set_gpio_addr(strtoull(argv[1],NULL,0)) == -1) {
				FAIL("HWCONFIG_SETGPIOADDR", "");
			}
			argc -= 2; argv += 2;
			continue;

#ifdef CLUSTER_SCHED
		} else if (0 == strcmp(argv[0], "--cluster_sched")) {
			if (argc < 2) die_usage();
			cluster_sched = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
#endif

		} else if (0 == strcmp(argv[0], "--help")) {
			kernel_setup();
			print_infos();
			usage();
			exit(0);

			/* Per-VM options */

		} else if (0 == strcmp(argv[0],"--ccr")) {
			if (argc < 2) die_usage();
			vm_params[idx].ccr = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
		} else if (0 == strcmp(argv[0],"--vwctrl")) {
			if (argc < 2) die_usage();
			vm_params[idx].vwctrl = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
#endif

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

		} else if (0 == strcmp(argv[0], "--reboot_reload")) {
			if (argc < 2) die_usage();
			vm_params[idx].reboot_reload = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--stash")) {
			if (argc < 2) die_usage();
			vm_params[idx].stash = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--stash_addr")) {
			if (argc < 2) die_usage();
			vm_params[idx].stash_addr = strtoul(argv[1],NULL,0);
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

		} else if (0 == strcmp(argv[0], "--pmu_sweep")) {
			if (argc < 2) die_usage();
			vm_params[idx].pmu_sweep = strtoul(argv[1],NULL,0);
			set_pmu_cfg = set_pmu_evtcfg1 = set_pmu_evtcfg = 1;
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_first_event")) {
			if (argc < 2) die_usage();
			vm_params[idx].pmu_first_event = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_last_event")) {
			if (argc < 2) die_usage();
			vm_params[idx].pmu_last_event = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--pmu_overflow")) {
			if (argc < 2) die_usage();
			vm_params[idx].pmu_overflow = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--sys_write_mode")) {
			if (argc < 2) die_usage();
			vm_params[idx].sys_write_mode = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;

		} else if (0 == strcmp(argv[0], "--va_angel")) {
			if (argc < 2) die_usage();
			vm_params[idx].va_angel = strtoul(argv[1],NULL,0);
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
	int i;

	int idx;

	pthread_init();

	//Remove booter from cmdline
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
		BOOTER_PRINTF("\n");
		BOOTER_PRINTF("Kernel error: %s\n\n", kerror_msg[kerror]);
		print_infos();
		return 1;
	}

	boot_flags.raw = h2_info(INFO_BOOT_FLAGS);
	stlb_info.raw = h2_info(INFO_STLB);

	tcm_base = (unsigned int)h2_info(INFO_TCM_BASE);
	tcm_size = h2_info(INFO_TCM_SIZE);
	clade_base = h2_info(INFO_CLADE_BASE);
	guest_base = H2K_GUEST_START;

	h2_galloc_init(&tcm_alloc, (unsigned int)tcm_base, (unsigned int)tcm_size, NULL);

	h2_vmtrap_setvec(bootvm_vectors);

	h2_anysignal_init(&wake_sig);

	if (h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_VM_CHILDINT, 0) < 0) {
		FAIL("H2K_INTOP_GLOBEN, H2K_VM_CHILDINT", "");
	}
	if (h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0) < 0) {
		FAIL("H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT", "");
	}
	h2_vmtrap_setie(1);

#ifdef MULTICORE
	core_id = h2_info(INFO_CORE_ID);
	core_count = h2_info(INFO_CORE_COUNT);
#endif

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

			kernel_setup();
			print_infos();
			run(idx);
			free(vm_params);
			vm_params = NULL;  // malloc anew if more --files
			tight_fence_hi = 1;
			guest_base = H2K_GUEST_START;

			h2_anysignal_init(&wake_sig);  // could be leftovers

		} else {  // not reading from file
			idx = 0;
			add_vm(idx);
			idx = process_line(argc, argv, idx);

			kernel_setup();
			print_infos();
			run(idx);
			if (ext_power) {
				if (h2_hwconfig_extpower(0) < 0) {
					FAIL("extpower", "");
				}
			}
			for (i = 0; i < getl2reg; i++) {
				get_l2_reg(i);
			}
			return 0;
		}
	}
	if (ext_power) {
		if (h2_hwconfig_extpower(0) < 0) {
			FAIL("extpower", "");
		}
	}
	for (i = 0; i < getl2reg; i++) {
		get_l2_reg(i);
	}
	return 0;
}

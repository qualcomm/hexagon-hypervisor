/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>
#include <runlist.h>
#include <readylist.h>
#include <lowprio.h>
#include <futex.h>
#include <intconfig.h>
#include <thread.h>
#include <setjmp.h>
#include <globals.h>
#include <sample.h>

jmp_buf env;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

u32_t TH_init_seen;
u32_t TH_switch_seen;

enum {
	runlist_init = 0,
	readylist_init,
	lowprio_init,
	futex_init,
	intconfig_init,
	kg_init,
	trace_init,
	timer_init,
	//	mem_alloc_init,
	//	tmpmap_init,
	l2cache_init,
	sample_init,
	//thread_init,
	//asid_table_init,
	//mem_stlb_init,
	tcm_copy,
	hvx_init,
	hmx_init,

#ifdef CRASH_DEBUG
	stlb_tcmcrash_init,
#endif

	XX_LAST_HELPER
};

void H2K_traptab()
{
}

u64_t H2K_stacks;

void H2K_interrupt_restore()
{
}

void H2K_start_threads(unsigned int mask)
{
}

H2K_thread_context a;
H2K_vmblock_t vmb;  // so we don't have to call the allocator

u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, u32_t val4, H2K_thread_context *me)
{
	static int done = 0;
	if (done) return 1;

	done = 1;
	H2K_vmblock_t *block = &vmb;
	block->free_threads = &a;
	block->contexts = &a;
	block->trapmask = ~0;
	H2K_gp->vmblocks[1] = block;
	return 1;
}

#define HELPER_FUNC(X) void H2K_##X() { TH_init_seen |= 1<< X; }

HELPER_FUNC(runlist_init)
HELPER_FUNC(readylist_init)
HELPER_FUNC(lowprio_init)
HELPER_FUNC(futex_init)
//HELPER_FUNC(intconfig_init)
void H2K_intconfig_init(u32_t ssbase) { TH_init_seen |= 1<< intconfig_init; }
HELPER_FUNC(trace_init)
HELPER_FUNC(timer_init)
//HELPER_FUNC(mem_alloc_init)
//HELPER_FUNC(tmpmap_init)
HELPER_FUNC(l2cache_init)
HELPER_FUNC(sample_init)
//HELPER_FUNC(thread_init)
//HELPER_FUNC(asid_table_init)

#ifdef CRASH_DEBUG
HELPER_FUNC(stlb_tcmcrash_init)
#endif

//HELPER_FUNC(mem_stlb_init)
HELPER_FUNC(tcm_copy)
HELPER_FUNC(hvx_init)
HELPER_FUNC(hmx_init)

	void H2K_kg_init(u32_t phys_offset, u32_t multicore_shift, u32_t devpage_offset, u32_t last_tlb_index, u32_t tlb_size, u32_t core_id, u32_t core_count, u32_t tcm_offset) { TH_init_seen |= 1<< kg_init; }

H2K_thread_context *boot;

/* We need to use a longjmp at the end, because H2K_switch is defined as
 * noreturn */
void H2K_switch(void *from, void *to)
{
	H2K_thread_context *expected = &H2K_kg.vmblocks[1]->contexts[MAX_BOOT_CONTEXTS - 1];
	if (from != NULL) FAIL("Unexpected switch call");
	printf("from=%p to=%p context=%p\n",from,to,expected);
	if (to != expected) FAIL("switch to non-boot thread");
	boot = to;
	TH_switch_seen = 1;
	longjmp(env,1);
}

void H2K_trace(s8_t type, u8_t hwtnum, u8_t tid, u32_t pcyclelo)
{
}

void H2K_cluster_config(void) {
}

H2K_kg_t H2K_kg;

void H2K_thread_boot();

int main()
{
	u32_t i;
	u32_t found_thread;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	H2K_gp->logbuf_enable = 0;

	H2K_gp->hthreads = get_hthreads();

	for (i = 0; i < H2K_gp->hthreads; i++) {
		H2K_gp->runlist[i] = 0;
	}
	TH_init_seen = 0;
	TH_switch_seen = 0;
	if (setjmp(env) == 0) {
		H2K_thread_boot();
	}
	if (TH_switch_seen == 0) FAIL("Did not switch to boot thread");
	for (i = 0; i < XX_LAST_HELPER; i++) {
		printf("%d\n",i);
		if (((1<<i) & TH_init_seen) == 0) FAIL("Didn't call init func");
	}
	if (boot->continuation != (H2K_interrupt_restore)) FAIL("Incorrect continuation");
	if (boot->trapmask != 0xffffffffU) FAIL("boot thread trapmask");
	found_thread = 0;
	for (i = 0; i < H2K_gp->hthreads; i++) {
		if (H2K_gp->runlist[i] == boot) {
			if (H2K_gp->runlist_prios[i] != 0) FAIL("Didn't push into runlist (0)");
			found_thread = 1;
		}
	}
	if (!found_thread) FAIL("Didn't push into runlist (1)");
	puts("TEST PASSED\n");
	exit(0);
}


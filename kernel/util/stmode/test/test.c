/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <max.h>
#include <stmode.h>
#include <hw.h>
#include <h2.h>
#include <context.h>
#include <globals.h>
#include <readylist.h>
#include <thread.h>
#include <check_sanity.h>
#include <asid.h>
#include <vm.h>
#include <id.h>
#include <hwconfig.h>

void H2K_interrupt_restore();

/* Local override of H2K_thread_create_no_squash: identical to the kernel version
   except ssr_guest/ssr_um are inherited from me instead of hardcoded to 1.
   This is needed because this test runs in monitor space (no BOOT/booter), so pages
   are supervisor-execute-only.  Created threads must run at the same privilege as
   main (ssr_um=0, ssr_guest=1) to be able to execute them without cause-0x11. */
IN_SECTION(".text.misc.create") s32_t H2K_thread_create_no_squash(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)
{
	H2K_thread_context *tmp;
	u32_t bestprio;
	u32_t trapmask;
	u32_t ptb;
	u32_t extra;
	translation_type type;
	s32_t asid;
	void *guest_evb;
	H2K_offset_t identity_offset = {
		.pages = 0,
		.cccc = 7,
		.size = 6,
		.xwru = 0xf,
	};

	bestprio = vmblock->bestprio;
	trapmask = vmblock->trapmask;

	if (vmblock->num_cpus == 0) {
		guest_evb = NULL;
		type = H2K_ASID_TRANS_TYPE_OFFSET;
		ptb = identity_offset.raw;
		extra = 0;
	} else {
		guest_evb = me->gevb;
		ptb = H2K_gp->asid_table[me->ssr_asid].ptb;
		type = H2K_gp->asid_table[me->ssr_asid].fields.type;
		extra = H2K_gp->asid_table[me->ssr_asid].fields.extra;
	}

	if (prio > MAX_PRIO) return -1;
	if (prio < bestprio) return -1;
	if ((sp & 7) != 0) return -1;
	if ((pc & 3) != 0) return -1;

	BKL_LOCK(&H2K_bkl);
	if (vmblock->free_threads == NULL) {
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}
	tmp = vmblock->free_threads;
	vmblock->free_threads = vmblock->free_threads->next;
	tmp->base_prio = tmp->prio = (u8_t)prio;
	tmp->gp = H2K_get_gp();
	tmp->usr = me->usr;
	tmp->ssr = me->ssr;
#if ARCHV >= 73
	tmp->vwctrl = me->vwctrl;
#endif
	tmp->elr = pc;
	tmp->r29 = sp;
	tmp->r0100 = arg1;
	tmp->ccr = H2K_get_ccr();
	tmp->trapmask = trapmask;
	tmp->tlbidxmask = vmblock->tlbidxmask;
	tmp->continuation = H2K_interrupt_restore;
	tmp->vmstatus = 0x0;
	tmp->gevb = guest_evb;

	asid = H2K_asid_table_inc(ptb, type, H2K_ASID_TLB_INVALIDATE_FALSE, extra, vmblock);
	if (asid == -1) {
		vmblock->free_threads = tmp;
		BKL_UNLOCK(&H2K_bkl);
		return -1;
	}

	/* inherit caller's privilege instead of hardcoding ssr_um=1/ssr_guest=1 */
	tmp->ssr_guest = me->ssr_guest;
	tmp->ssr_um    = me->ssr_um;
	tmp->ssr_asid  = (u8_t)asid;
#ifdef HAVE_EXTENSIONS
	tmp->ssr_xa  = EXT_NO_EXT;
	tmp->ssr_xe  = 0;
	tmp->ssr_xe2 = 0;
	tmp->ccr_xe3 = 0;
#endif

	vmblock->num_cpus++;
	tmp->vmblock = vmblock;

	H2K_ready_append(tmp);
	return (s32_t)H2K_check_sanity_unlock(H2K_id_from_context(tmp).raw);
}

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

volatile int handshake = 0;

#define STACK_SIZE 16
#define NUM_TEST_THREADS 2  /* test1 + test2 */

unsigned long long int stack1[STACK_SIZE];
unsigned long long int stack2[STACK_SIZE];

/* Extra thread contexts for test1/test2.  The boot VM is created with
   MAX_BOOT_CONTEXTS=1 (just main), leaving no free slots.  We extend
   the free list with two statically-allocated contexts before creating
   any threads. */
static H2K_thread_context extra_contexts[NUM_TEST_THREADS] __attribute__((aligned(32)));

static inline void stmode_extend_bootvm_contexts(H2K_thread_context *me)
{
	H2K_vmblock_t *vm = me->vmblock;
	int i;
	for (i = 0; i < NUM_TEST_THREADS; i++) {
		H2K_thread_context_clear(&extra_contexts[i]);
		extra_contexts[i].id.raw = 0;
		extra_contexts[i].id.vmidx = vm->vmidx;
		extra_contexts[i].id.cpuidx = vm->max_cpus++;
		extra_contexts[i].vmblock = vm;
		extra_contexts[i].next = vm->free_threads;
		vm->free_threads = &extra_contexts[i];
	}
}

static inline void wait_for_threads_to_sleep(void)
{
	u32_t mc;
	do {
		mc = H2K_get_modectl();
	} while ((mc & ((~mc) >> MODECTL_W_BITS)) & ~0x1u);
}

void test1(void *unused)
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	while (1) {
		handshake = 1;
		asm volatile (" wait(r0)\n");
	}
}

void test2(void *unused)
{
	u32_t tmp;
	asm volatile (" %0 = #-1; imask=%0 " : "=r"(tmp));
	while (1) {
		handshake = 1;
		asm volatile (" nop\n");
	}
}

int main()
{
	h2_init(NULL);
	H2K_thread_context *me = H2K_get_sgp();
	u32_t tmp;

	stmode_extend_bootvm_contexts(me);
	H2K_trap_hwconfig_hwthreads_mask(0, NULL, (u32_t)-1, 0, me);
	wait_for_threads_to_sleep();

	H2K_set_gie();

	tmp = H2K_stmode_begin();
	if (tmp != 0) FAIL("stmode failed (1 thread active)");
	if ((H2K_get_syscfg() & 0x10) != 0) FAIL("stmode_begin didn't disable gie");
	H2K_stmode_end();
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_end didn't ensable gie");

	h2_thread_create(test1, &stack1[STACK_SIZE], (void *)1, 0);
	while (handshake == 0) /* SPIN */;
	for (tmp = 0; tmp < 100; tmp++) { asm volatile ("nop"); }

	tmp = H2K_stmode_begin();
	if (tmp != 0) FAIL("stmode failed (1 thread sleep)");
	if ((H2K_get_syscfg() & 0x10) != 0) FAIL("stmode_begin didn't disable gie");
	H2K_stmode_end();
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_end didn't ensable gie");

	handshake = 0;

	h2_thread_create(test2, &stack2[STACK_SIZE], (void *)2, 0);
	while (handshake == 0) /* SPIN */;
	for (tmp = 0; tmp < 100; tmp++) { asm volatile ("nop"); }

	tmp = H2K_stmode_begin();
	if (tmp == 0) FAIL("stmode passed (2 threads active)");
	if ((H2K_get_syscfg() & 0x10) == 0) FAIL("stmode_begin didn't reenable gie");

	puts("TEST PASSED");
	return 0;
}
